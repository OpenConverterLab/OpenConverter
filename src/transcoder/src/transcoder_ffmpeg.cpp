/*
 * Copyright 2025 Jack Lau
 * Email: jacklau1222gm@gmail.com
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 */

#include "../include/transcoder_ffmpeg.h"
extern "C" {
#include <libavutil/pixdesc.h>
}
#include <chrono>

/* Receive pointers from converter */
TranscoderFFmpeg::TranscoderFFmpeg(ProcessParameter *processParameter,
                                   EncodeParameter *encodeParameter)
    : Transcoder(processParameter, encodeParameter) {
    frameTotalNumber = 0;
    total_duration = 0;
    current_duration = 0;
}

void TranscoderFFmpeg::print_error(const char *msg, int ret) {
    av_strerror(ret, errorMsg, sizeof(errorMsg));
    av_log(NULL, AV_LOG_ERROR, " %s: %s \n", msg, errorMsg);
}

void TranscoderFFmpeg::update_progress(int64_t current_pts,
                                       AVRational time_base) {
    // Convert current PTS to microseconds
    AVRational micros_base = {1, 1000000};
    current_duration = av_rescale_q(current_pts, time_base, micros_base);

    // Calculate progress percentage
    if (total_duration > 0) {
        // Use the base class's send_process_parameter which handles time delays
        // and smoothing
        send_process_parameter(current_duration, total_duration);
    }
}

int TranscoderFFmpeg::init_filter(AVCodecContext *dec_ctx, FilteringContext *filter_ctx, const char *filters_descr)
{
    char args[512];
    int ret = 0;
    const AVFilter *buffersrc = NULL;
    const AVFilter *buffersink = NULL;
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    AVFilterContext *buffersink_ctx = NULL;
    AVFilterContext *buffersrc_ctx = NULL;
    AVFilterGraph *filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
        buffersrc  = avfilter_get_by_name("buffer");
        buffersink = avfilter_get_by_name("buffersink");
        /* buffer video source: the decoded frames from the decoder will be inserted here. */
        snprintf(args, sizeof(args),
                "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
                dec_ctx->time_base.num, dec_ctx->time_base.den,
                dec_ctx->sample_aspect_ratio.num, dec_ctx->sample_aspect_ratio.den);
    } else if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
        char buf[64];
        buffersrc  = avfilter_get_by_name("abuffer");
        buffersink = avfilter_get_by_name("abuffersink");
        if (dec_ctx->ch_layout.order == AV_CHANNEL_ORDER_UNSPEC)
            av_channel_layout_default(&dec_ctx->ch_layout, dec_ctx->ch_layout.nb_channels);
        av_channel_layout_describe(&dec_ctx->ch_layout, buf, sizeof(buf));
        snprintf(args, sizeof(args),
            "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=%s",
            dec_ctx->time_base.num, dec_ctx->time_base.den, dec_ctx->sample_rate,
            av_get_sample_fmt_name(dec_ctx->sample_fmt),
            buf);
    } else {
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    if (!buffersrc || !buffersink) {
        av_log(NULL, AV_LOG_ERROR, "filtering source or sink element not found\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }


    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
                                       args, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        goto end;
    }

    /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                       NULL, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
        goto end;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */

    /*
     * The buffer source output must be connected to the input pad of
     * the first filter described by filters_descr; since the first
     * filter input label is not specified, it is set to "in" by
     * default.
     */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
                                    &inputs, &outputs, NULL)) < 0)
        goto end;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto end;

    filter_ctx->buffersink_ctx = buffersink_ctx;
    filter_ctx->buffersrc_ctx = buffersrc_ctx;
    filter_ctx->filter_graph = filter_graph;

end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}


int TranscoderFFmpeg::init_filters_wrapper(StreamContext *decoder)
{
    int i, ret;
    const char *filters_descr;
    AVCodecContext *dec_ctx = NULL;
    filters_ctx = reinterpret_cast<FilteringContext *>(av_malloc_array(decoder->fmtCtx->nb_streams, sizeof(*filters_ctx)));
    if (!filters_ctx)
        return AVERROR(ENOMEM);

    for (i = 0; i < decoder->fmtCtx->nb_streams; i++) {
        filters_ctx[i].buffersrc_ctx  = NULL;
        filters_ctx[i].buffersink_ctx = NULL;
        filters_ctx[i].filter_graph   = NULL;
        if (!(decoder->fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO ||
            decoder->fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO))
            continue;
        if (decoder->fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            std::string d = "";
            std::string pixelFormat = encodeParameter->get_pixel_format();
            uint16_t width = encodeParameter->get_width();
            uint16_t height = encodeParameter->get_height();
            if (!pixelFormat.empty()) {
                d += "format=" + pixelFormat;
            }
            if (width > 0 && height > 0) {
                if (!d.empty()) {
                    d += ",";
                }
                d += "scale=" + std::to_string(width) + ":" + std::to_string(height);
            }
            if (d.empty())
                d = "null";
            filters_descr = d.c_str();
            dec_ctx = decoder->videoCodecCtx;
        } else {
            filters_descr = "anull";
            dec_ctx = decoder->audioCodecCtx;
        }
        ret = init_filter(dec_ctx, &filters_ctx[i], filters_descr);
    }
    return ret;
}

bool TranscoderFFmpeg::transcode(std::string input_path,
                                 std::string output_path) {
    bool flag = false;
    int ret = -1;
    // deal with arguments

    StreamContext *decoder = new StreamContext;
    StreamContext *encoder = new StreamContext;

    // Declare variables before any goto statements
    double startTime = encodeParameter->GetStartTime();
    double endTime = encodeParameter->GetEndTime();
    int64_t endPts = -1;

    av_log_set_level(AV_LOG_DEBUG);

    decoder->filename = input_path.c_str();
    encoder->filename = output_path.c_str();

    if (encodeParameter->get_video_codec_name() == "") {
        copyVideo = true;
    } else {
        copyVideo = false;
    }

    if (encodeParameter->get_audio_codec_name() == "") {
        copyAudio = true;
    } else {
        copyAudio = false;
    }

    if ((ret = open_media(decoder, encoder)) < 0)
        goto end;

    // Calculate total duration from the input file
    if (decoder->fmtCtx->duration != AV_NOPTS_VALUE) {
        total_duration = decoder->fmtCtx->duration;
    } else {
        // If duration is not available, try to get it from streams
        for (unsigned int i = 0; i < decoder->fmtCtx->nb_streams; i++) {
            AVStream *stream = decoder->fmtCtx->streams[i];
            if (stream->duration != AV_NOPTS_VALUE) {
                AVRational micros_base = {1, 1000000};
                int64_t stream_duration = av_rescale_q(
                    stream->duration, stream->time_base, micros_base);
                total_duration = std::max(total_duration, stream_duration);
            }
        }
    }

    // If we still don't have a duration, try to estimate from stream properties
    if (total_duration == 0) {
        for (unsigned int i = 0; i < decoder->fmtCtx->nb_streams; i++) {
            AVStream *stream = decoder->fmtCtx->streams[i];
            if (stream->nb_frames > 0 && stream->avg_frame_rate.num > 0) {
                AVRational micros_base = {1, 1000000};
                int64_t estimated_duration =
                    av_rescale_q(stream->nb_frames * stream->avg_frame_rate.den,
                                 stream->avg_frame_rate, micros_base);
                total_duration = std::max(total_duration, estimated_duration);
            }
        }
    }

    if ((ret = prepare_decoder(decoder)) < 0)
        goto end;

    if ((ret = init_filters_wrapper(decoder)) < 0)
        goto end;

    for (int i = 0; i < decoder->fmtCtx->nb_streams; i++) {
        if (decoder->fmtCtx->streams[i]->codecpar->codec_type ==
            AVMEDIA_TYPE_VIDEO) {
            // skip video streams
            if (encoder->fmtCtx->oformat->video_codec == AV_CODEC_ID_NONE) {
                continue;
            }
            if (!copyVideo) {
                if ((ret = prepare_encoder_video(decoder, encoder)) < 0)
                    goto end;
            } else {
                ret = prepare_copy(encoder->fmtCtx, &encoder->videoStream,
                                   decoder->videoStream->codecpar);
                if (ret < 0)
                    goto end;
            }
        } else if (decoder->fmtCtx->streams[i]->codecpar->codec_type ==
                   AVMEDIA_TYPE_AUDIO) {
            // skip audio streams
            if (encoder->fmtCtx->oformat->audio_codec == AV_CODEC_ID_NONE) {
                continue;
            }
            if (!copyAudio) {
                if ((ret = prepare_encoder_audio(decoder, encoder)) < 0)
                    goto end;
            } else {
                ret = prepare_copy(encoder->fmtCtx, &encoder->audioStream,
                                   decoder->audioStream->codecpar);
                if (ret < 0)
                    goto end;
            }
        }
    }
    // binding
    ret = avio_open2(&encoder->fmtCtx->pb, encoder->filename, AVIO_FLAG_WRITE,
                     NULL, NULL);
    if (ret < 0) {
        print_error("Failed to open output file", ret);
        goto end;
    }
    /* Write the stream header, if any. */
    if ((ret = avformat_write_header(encoder->fmtCtx, NULL)) < 0) {
        print_error("Failed to write header", ret);
        goto end;
    }

    // Handle start time seeking if specified
    if (startTime > 0) {
        int64_t seek_target = static_cast<int64_t>(startTime * AV_TIME_BASE);
        if ((ret = av_seek_frame(decoder->fmtCtx, -1, seek_target, AVSEEK_FLAG_BACKWARD)) < 0) {
            av_log(NULL, AV_LOG_WARNING, "Could not seek to start time\n");
        }
        // Flush codec buffers after seeking
        if (decoder->videoCodecCtx) {
            avcodec_flush_buffers(decoder->videoCodecCtx);
        }
        if (decoder->audioCodecCtx) {
            avcodec_flush_buffers(decoder->audioCodecCtx);
        }
    }

    // Calculate end time in stream time base for comparison
    if (endTime > 0 && decoder->videoIdx >= 0) {
        endPts = static_cast<int64_t>(endTime / av_q2d(decoder->videoStream->time_base));
    }

    // read video data from multimedia files to write into destination file
    while (av_read_frame(decoder->fmtCtx, decoder->pkt) >= 0) {
        // Check if we've reached the end time
        if (endPts > 0 && decoder->pkt->stream_index == decoder->videoIdx) {
            if (decoder->pkt->pts >= endPts) {
                av_packet_unref(decoder->pkt);
                break;
            }
        }

        if (decoder->pkt->stream_index == decoder->videoIdx) {
            if (encoder->fmtCtx->oformat->video_codec == AV_CODEC_ID_NONE) {
                continue;
            }

            // Skip frames before start time
            if (startTime > 0) {
                double framePts = decoder->pkt->pts * av_q2d(decoder->videoStream->time_base);
                if (framePts < startTime) {
                    av_packet_unref(decoder->pkt);
                    continue;
                }
            }

            // Update progress based on video stream
            update_progress(decoder->pkt->pts, decoder->videoStream->time_base);

            if (!copyVideo) {
                av_packet_rescale_ts(decoder->pkt, decoder->videoStream->time_base,
                                     decoder->videoCodecCtx->time_base);
                if ((ret = transcode_video(decoder, encoder)) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Failed to transcode video frame\n");
                    goto end;
                }
            } else {
                ret = remux(decoder->pkt, encoder->fmtCtx, decoder->videoStream,
                            encoder->videoStream);
                if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Failed to remux video packet\n");
                    goto end;
                }
            }
        } else if (decoder->pkt->stream_index == decoder->audioIdx) {
            if (encoder->fmtCtx->oformat->audio_codec == AV_CODEC_ID_NONE) {
                continue;
            }

            // Skip frames before start time
            if (startTime > 0) {
                double framePts = decoder->pkt->pts * av_q2d(decoder->audioStream->time_base);
                if (framePts < startTime) {
                    av_packet_unref(decoder->pkt);
                    continue;
                }
            }

            // Update progress based on audio stream if no video stream
            if (decoder->videoIdx < 0) {
                update_progress(decoder->pkt->pts,
                                decoder->audioStream->time_base);
            }

            if (!copyAudio) {
                av_packet_rescale_ts(decoder->pkt, decoder->audioStream->time_base,
                                     decoder->audioCodecCtx->time_base);
                if ((ret = transcode_audio(decoder, encoder)) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Failed to transcode audio frame\n");
                    goto end;
                }
            } else {
                ret = remux(decoder->pkt, encoder->fmtCtx, decoder->audioStream,
                            encoder->audioStream);
                if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Failed to remux audio packet\n");
                    goto end;
                }
            }
        }
    }
    if (!copyVideo) {
        encoder->frame = NULL;
        // write the buffered frame
        if ((ret = encode_video(decoder->videoStream, encoder, NULL)) < 0) {
            av_log(NULL, AV_LOG_ERROR, "Failed to flush video encoder\n");
            goto end;
        }
    }

    processParameter->set_process_number(1, 1);

    if ((ret = av_write_trailer(encoder->fmtCtx)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to write trailer");
        goto end;
    }

    flag = true;
// free memory
end:
    if (decoder->fmtCtx) {
        avformat_close_input(&decoder->fmtCtx);
        decoder->fmtCtx = NULL;
    }
    delete decoder;

    if (encoder->fmtCtx && !(encoder->fmtCtx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&encoder->fmtCtx->pb);
    }
    delete encoder;

    return flag;
}

int TranscoderFFmpeg::open_media(StreamContext *decoder,
                                 StreamContext *encoder) {
    int ret = -1;
    /* set the frameNumber to zero to avoid some bugs */
    frameNumber = 0;
    // open the multimedia file
    if ((ret = avformat_open_input(&decoder->fmtCtx, decoder->filename, NULL,
                                   NULL)) < 0) {
        print_error("Failed to open input file", ret);
        return ret;
    }

    if ((ret = avformat_find_stream_info(decoder->fmtCtx, NULL)) < 0) {
        print_error("Failed to find stream info", ret);
        return ret;
    }

    ret = avformat_alloc_output_context2(&encoder->fmtCtx, NULL, NULL,
                                         encoder->filename);
    if (!encoder->fmtCtx) {
        av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
        return AVERROR(ENOMEM);
    }

    return 0;
}

int TranscoderFFmpeg::encode_video(AVStream *inStream, StreamContext *encoder,
                                   AVFrame *inputFrame) {
    int ret = -1;
    AVPacket *output_packet = av_packet_alloc();
    FilteringContext *fc = &filters_ctx[inStream->index];

    /* push the decoded frame into the filtergraph */
    if ((ret = av_buffersrc_add_frame_flags(fc->buffersrc_ctx, inputFrame, AV_BUFFERSRC_FLAG_KEEP_REF)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
        goto end;
    }
    /* pull filtered frames from the filtergraph */
    while (1) {
        if ((ret = av_buffersink_get_frame(fc->buffersink_ctx, inputFrame)) == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        if (ret < 0)
            goto end;
    }

    if (encodeParameter->get_qscale() != -1 && inputFrame) {
        inputFrame->quality = encoder->videoCodecCtx->global_quality;
        inputFrame->pict_type = AV_PICTURE_TYPE_NONE;
    }
    // send frame to encoder
    if ((ret = avcodec_send_frame(encoder->videoCodecCtx, inputFrame)) < 0) {
        print_error("Failed to send frame to encoder", ret);
        goto end;
    }

    while (ret >= 0) {
        if ((ret = avcodec_receive_packet(encoder->videoCodecCtx, output_packet)) == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            ret = 0;
            goto end;
        } else if (ret < 0) {
            goto end;
        }

        output_packet->stream_index = encoder->videoStream->index;

        av_packet_rescale_ts(output_packet, encoder->videoCodecCtx->time_base,
                             encoder->videoStream->time_base);

        if ((ret = av_interleaved_write_frame(encoder->fmtCtx, output_packet)) < 0) {
            print_error("Failed to write packet", ret);
        }

        av_packet_unref(output_packet);
    }

end:
    av_packet_free(&output_packet);
    return ret;
}

int TranscoderFFmpeg::encode_audio(AVStream *in_stream, StreamContext *encoder,
                                   AVFrame *input_frame) {
    int ret = -1;
    AVPacket *output_packet = av_packet_alloc();
    FilteringContext *fc = &filters_ctx[in_stream->index];

    /* push the decoded frame into the filtergraph */
    if ((ret = av_buffersrc_add_frame_flags(fc->buffersrc_ctx, input_frame, AV_BUFFERSRC_FLAG_KEEP_REF)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
        goto end;
    }
    /* pull filtered frames from the filtergraph */
    while (1) {
        if ((ret = av_buffersink_get_frame(fc->buffersink_ctx, input_frame)) == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        if (ret < 0)
            goto end;
    }
    // send frame to encoder
    if ((ret = avcodec_send_frame(encoder->audioCodecCtx, input_frame)) < 0) {
        print_error("Failed to send frame to encoder", ret);
        goto end;
    }
    while (ret >= 0) {
        if ((ret = avcodec_receive_packet(encoder->audioCodecCtx, output_packet)) == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            ret = 0;
            goto end;
        } else if (ret < 0) {
            goto end;
        }
        output_packet->stream_index = encoder->audioStream->index;
        av_packet_rescale_ts(output_packet, encoder->audioCodecCtx->time_base,
                             encoder->audioStream->time_base);
        if ((ret = av_interleaved_write_frame(encoder->fmtCtx, output_packet)) < 0) {
            print_error("Failed to write packet", ret);
        }
        av_packet_unref(output_packet);
    }

end:
    av_packet_free(&output_packet);
    return ret;
}

int TranscoderFFmpeg::transcode_video(StreamContext *decoder,
                                      StreamContext *encoder) {
    int ret = -1;

    // send packet to decoder
    if ((ret = avcodec_send_packet(decoder->videoCodecCtx, decoder->pkt)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to send packet to decoder!\n");
        goto end;
    }

    while (ret >= 0) {
        if ((ret = avcodec_receive_frame(decoder->videoCodecCtx, decoder->frame)) == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            ret = 0;
            goto end;
        } else if (ret < 0) {
            goto end;
        }

        if ((ret = encode_video(decoder->videoStream, encoder, decoder->frame)) < 0) {
            goto end;
        }

        if (decoder->pkt) {
            av_packet_unref(decoder->pkt);
        }

        av_frame_unref(decoder->frame);
    }

end:
    return ret;
}

int TranscoderFFmpeg::transcode_audio(StreamContext *decoder,
                                      StreamContext *encoder) {
    int ret;
    if ((ret = avcodec_send_packet(decoder->audioCodecCtx, decoder->pkt)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to send packet to decoder!\n");
        return ret;
    }

    while (ret >= 0) {
        if ((ret = avcodec_receive_frame(decoder->audioCodecCtx, decoder->frame)) == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            ret = 0;
            break;
        } else if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR,
                   "Failed to receive frame from decoder!\n");
            return ret;
        }

        if ((ret = encode_audio(decoder->audioStream, encoder, decoder->frame)) < 0) {
            return ret;
        }

        if (decoder->pkt) {
            av_packet_unref(decoder->pkt);
        }
        av_frame_unref(decoder->frame);
    }
    return ret;
}

int TranscoderFFmpeg::prepare_decoder(StreamContext *decoder) {
    int ret = -1;

    for (int i = 0; i < decoder->fmtCtx->nb_streams; i++) {
        if (decoder->fmtCtx->streams[i]->codecpar->codec_type ==
            AVMEDIA_TYPE_VIDEO) {
            decoder->videoStream = decoder->fmtCtx->streams[i];
            decoder->videoIdx = i;
        } else if (decoder->fmtCtx->streams[i]->codecpar->codec_type ==
                   AVMEDIA_TYPE_AUDIO) {
            decoder->audioStream = decoder->fmtCtx->streams[i];
            decoder->audioIdx = i;
        }
    }

    // find the decoder
    // decoder->videoCodec = avcodec_find_encoder_by_name(codec);
    // find the decoder by ID
    if (decoder->videoIdx != OC_INVALID_STREAM_IDX) {
        decoder->videoCodec =
            avcodec_find_decoder(decoder->videoStream->codecpar->codec_id);
        if (!decoder->videoCodec) {
            av_log(NULL, AV_LOG_ERROR, "Couldn't find codec: %s \n",
                avcodec_get_name(decoder->videoStream->codecpar->codec_id));
            return AVERROR_DECODER_NOT_FOUND;
        }
        // init decoder context
        decoder->videoCodecCtx = avcodec_alloc_context3(decoder->videoCodec);
        if (!decoder->videoCodecCtx) {
            av_log(NULL, AV_LOG_ERROR, "No memory!\n");
            return AVERROR(ENOMEM);
        }
        avcodec_parameters_to_context(decoder->videoCodecCtx,
                                    decoder->videoStream->codecpar);
        decoder->videoCodecCtx->framerate = av_guess_frame_rate(decoder->fmtCtx, decoder->videoStream, NULL);
        // bind decoder and decoder context
        if ((ret = avcodec_open2(decoder->videoCodecCtx, decoder->videoCodec, NULL)) < 0) {
            print_error("Couldn't open the codec", ret);
            return ret;
        }
    }

    if (decoder->audioIdx != OC_INVALID_STREAM_IDX) {
        decoder->audioCodec =
            avcodec_find_decoder(decoder->audioStream->codecpar->codec_id);
        if (!decoder->audioCodec) {
            av_log(NULL, AV_LOG_ERROR, "Couldn't find codec: %s \n",
                    avcodec_get_name(decoder->audioStream->codecpar->codec_id));
            return AVERROR_DECODER_NOT_FOUND;
        }
        decoder->audioCodecCtx = avcodec_alloc_context3(decoder->audioCodec);
        if (!decoder->audioCodecCtx) {
            av_log(NULL, AV_LOG_ERROR, "No Memory!");
            return AVERROR(ENOMEM);
        }
        avcodec_parameters_to_context(decoder->audioCodecCtx,
                                    decoder->audioStream->codecpar);


        if ((ret = avcodec_open2(decoder->audioCodecCtx, decoder->audioCodec, NULL)) < 0) {
            print_error("Couldn't open the codec", ret);
            return ret;
        }
    }

    // create AVFrame
    decoder->frame = av_frame_alloc();
    if (!decoder->frame) {
        av_log(NULL, AV_LOG_ERROR, "No Memory!\n");
        return AVERROR(ENOMEM);
    }

    // create AVPacket
    decoder->pkt = av_packet_alloc();
    if (!decoder->pkt) {
        av_log(NULL, AV_LOG_ERROR, "NO Memory!\n");
        return AVERROR(ENOMEM);
    }

    return 0;
}

int TranscoderFFmpeg::prepare_encoder_video(StreamContext *decoder,
                                            StreamContext *encoder) {
    int ret = -1;

    /* set the total numbers of frame */
    frameTotalNumber = decoder->videoStream->nb_frames;
    /**
     * set the output file parameters
     */
    // find the encodec by Name
    //  QByteArray ba = encodeParamter->get_video_codec_name().toLocal8Bit();
    std::string codec = encodeParameter->get_video_codec_name();
    encoder->videoCodec = avcodec_find_encoder_by_name(codec.c_str());

    // find the encodec by ID
    // encoder->videoCodec =
    // avcodec_find_encoder(decoder->videoCodecCtx->codec_id);
    if (!encoder->videoCodec) {
        av_log(NULL, AV_LOG_ERROR, "Couldn't find video codec: %s\n",
               codec.c_str());
        return AVERROR_ENCODER_NOT_FOUND;
    }

    // init codec context
    encoder->videoCodecCtx = avcodec_alloc_context3(encoder->videoCodec);
    if (!encoder->videoCodecCtx) {
        av_log(NULL, AV_LOG_ERROR, "No memory!\n");
        return AVERROR(ENOMEM);
    }

    std::string preset = encodeParameter->get_preset();
    if (!preset.empty())
        av_opt_set(encoder->videoCodecCtx->priv_data, "preset", preset.c_str(), 0);

    if (decoder->videoCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO) {
        uint16_t width = encodeParameter->get_width();
        uint16_t height = encodeParameter->get_height();
        std::string pixelFormat = encodeParameter->get_pixel_format();
        AVRational tpf = {decoder->videoCodecCtx->ticks_per_frame, 1};
        if (width > 0)
            encoder->videoCodecCtx->width = width;
        else
            encoder->videoCodecCtx->width = decoder->videoCodecCtx->width;
        if (height > 0)
            encoder->videoCodecCtx->height = height;
        else
            encoder->videoCodecCtx->height = decoder->videoCodecCtx->height;

        if (encodeParameter->get_video_bit_rate())
            encoder->videoCodecCtx->bit_rate = encodeParameter->get_video_bit_rate();
        else
            encoder->videoCodecCtx->bit_rate = 0; // use default rate control(crf)
        encoder->videoCodecCtx->sample_aspect_ratio =
            decoder->videoCodecCtx->sample_aspect_ratio;
        // the AVCodecContext don't have framerate
        // outCodecCtx->time_base = av_inv_q(inCodecCtx->framerate);
        if (!pixelFormat.empty())
            encoder->videoCodecCtx->pix_fmt = av_get_pix_fmt(pixelFormat.c_str());
        else if (decoder->videoCodecCtx->pix_fmt != AV_PIX_FMT_NONE)
            encoder->videoCodecCtx->pix_fmt = decoder->videoCodecCtx->pix_fmt;
        else if (encoder->videoCodec->pix_fmts)
            encoder->videoCodecCtx->pix_fmt = encoder->videoCodec->pix_fmts[0];
        else
            encoder->videoCodecCtx->pix_fmt = AV_PIX_FMT_NONE;

        // encoder->videoCodecCtx->max_b_frames = 0;
        encoder->videoCodecCtx->time_base = av_inv_q(av_mul_q(decoder->videoCodecCtx->framerate, tpf));
        int qscale = encodeParameter->get_qscale();
        if (qscale != -1) {
            encoder->videoCodecCtx->flags |= AV_CODEC_FLAG_QSCALE;
            encoder->videoCodecCtx->global_quality = qscale * FF_QP2LAMBDA;
        }
    }

    if (encoder->fmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
        encoder->videoCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    // bind codec and codec context
    if ((ret = avcodec_open2(encoder->videoCodecCtx, encoder->videoCodec, NULL)) < 0) {
        print_error("Couldn't open the codec", ret);
        return ret;
    }

    encoder->videoStream = avformat_new_stream(encoder->fmtCtx, NULL);
    if (!encoder->videoStream) {
        av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
        return AVERROR(ENOMEM);
    }

    ret = avcodec_parameters_from_context(encoder->videoStream->codecpar,
                                          encoder->videoCodecCtx);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR,
               "Failed to copy encoder parameters to output stream #\n");
        return ret;
    }
    encoder->videoStream->time_base = encoder->videoCodecCtx->time_base;

    // oFmtCtx->oformat = av_guess_format(NULL, dst, NULL);
    // if(!oFmtCtx->oformat)
    // {
    //     av_log(NULL, AV_LOG_ERROR, "No Memory!\n");
    // }

    return 0;
}

int TranscoderFFmpeg::prepare_encoder_audio(StreamContext *decoder,
                                            StreamContext *encoder) {
    int ret = -1;
    /**
     * set the output file parameters
     */
    // find the encodec by name
    std::string codec = encodeParameter->get_audio_codec_name();
    encoder->audioCodec = avcodec_find_encoder_by_name(codec.c_str());
    if (!encoder->audioCodec) {
        av_log(NULL, AV_LOG_ERROR, "Couldn't find audio codec: %s\n",
               codec.c_str());
        return AVERROR_ENCODER_NOT_FOUND;
    }
    // init codec context
    encoder->audioCodecCtx = avcodec_alloc_context3(encoder->audioCodec);
    if (!encoder->audioCodecCtx) {
        av_log(NULL, AV_LOG_ERROR, "No memory!\n");
        return AVERROR(ENOMEM);
    }
    if (decoder->audioCodecCtx->codec_type == AVMEDIA_TYPE_AUDIO) {
#ifdef OC_FFMPEG_VERSION
    #if OC_FFMPEG_VERSION < 50
        encoder->audioCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    #else
        AVChannelLayout stereoLayout = AV_CHANNEL_LAYOUT_STEREO;
        av_channel_layout_copy(&encoder->audioCodecCtx->ch_layout,
                               &stereoLayout);
    #endif
#endif
        encoder->audioCodecCtx->sample_rate =
            decoder->audioCodecCtx->sample_rate;
        encoder->audioCodecCtx->sample_fmt =
            encoder->audioCodec->sample_fmts[0];
        if (encodeParameter->get_audio_bit_rate())
            encoder->audioCodecCtx->bit_rate = encodeParameter->get_audio_bit_rate();
        else
            encoder->audioCodecCtx->bit_rate = decoder->audioCodecCtx->bit_rate;
        encoder->audioCodecCtx->time_base =
            av_make_q(1, decoder->audioCodecCtx->sample_rate);
        encoder->audioCodecCtx->strict_std_compliance =
            FF_COMPLIANCE_EXPERIMENTAL;
    }
    // bind codec and codec context
    if ((ret = avcodec_open2(encoder->audioCodecCtx, encoder->audioCodec, NULL)) < 0) {
        print_error("Couldn't open the codec", ret);
        goto end;
    }
    encoder->audioStream = avformat_new_stream(encoder->fmtCtx, NULL);
    if (!encoder->audioStream) {
        av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }
    encoder->audioStream->time_base = encoder->audioCodecCtx->time_base;
    ret = avcodec_parameters_from_context(encoder->audioStream->codecpar,
                                          encoder->audioCodecCtx);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR,
               "Failed to copy encoder parameters to output stream #\n");
        goto end;
    }
end:
    return ret;
}

int TranscoderFFmpeg::prepare_copy(AVFormatContext *avCtx, AVStream **stream,
                                   AVCodecParameters *codecParam) {
    *stream = avformat_new_stream(avCtx, NULL);
    if (!*stream) {
        av_log(NULL, AV_LOG_ERROR, "Failed to allocate stream\n");
        return AVERROR(ENOMEM);
    }

    int ret = avcodec_parameters_copy((*stream)->codecpar, codecParam);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to copy codec parameters\n");
        return ret;
    }

    // the "mp4a" tag in MP4 is incompatible with MKV.
    // so set the codec_tag of the audio stream to 0 to avoid compatibility
    // issues. we will improve this method in the future.
    if (codecParam->codec_type == AVMEDIA_TYPE_AUDIO) {
        (*stream)->codecpar->codec_tag = 0;
    }
    return 0;
}

int TranscoderFFmpeg::remux(AVPacket *pkt, AVFormatContext *avCtx,
                            AVStream *inStream, AVStream *outStream) {
    // associate the avpacket with the target output avstream
    pkt->stream_index = outStream->index;
    av_packet_rescale_ts(pkt, inStream->time_base, outStream->time_base);
    int ret = av_interleaved_write_frame(avCtx, pkt);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "write frame error!\n");
        return ret;
    }
    return 0;
}

TranscoderFFmpeg::~TranscoderFFmpeg() {}
