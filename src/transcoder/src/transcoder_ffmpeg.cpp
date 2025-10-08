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
    filter_graph = nullptr;
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

int TranscoderFFmpeg::init_filters(StreamContext *decoder, const char *filters_descr)
{
    char args[512];
    int ret = 0;
    const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    AVRational time_base = decoder->fmtCtx->streams[decoder->videoIdx]->time_base;

    filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args),
            "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
            decoder->videoCodecCtx->width, decoder->videoCodecCtx->height, decoder->videoCodecCtx->pix_fmt,
            time_base.num, time_base.den,
            decoder->videoCodecCtx->sample_aspect_ratio.num, decoder->videoCodecCtx->sample_aspect_ratio.den);

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

end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}


bool TranscoderFFmpeg::init_filters_wrapper(StreamContext *decoder)
{
    std::string d = "";
    const char *filters_descr;
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
        return true;// No filters to apply
    filters_descr = d.c_str();
    return init_filters(decoder, filters_descr) < 0 ? false : true;
}

bool TranscoderFFmpeg::transcode(std::string input_path,
                                 std::string output_path) {
    bool flag = true;
    int ret = -1;
    // deal with arguments

    StreamContext *decoder = new StreamContext;
    StreamContext *encoder = new StreamContext;

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

    if (!open_media(decoder, encoder)) {
        flag = false;
        goto end;
    }

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

    if (!prepare_decoder(decoder)) {
        flag = false;
        goto end;
    }

    if (!init_filters_wrapper(decoder)) {
        flag = false;
        goto end;
    }

    for (int i = 0; i < decoder->fmtCtx->nb_streams; i++) {
        if (decoder->fmtCtx->streams[i]->codecpar->codec_type ==
            AVMEDIA_TYPE_VIDEO) {
            // skip video streams
            if (encoder->fmtCtx->oformat->video_codec == AV_CODEC_ID_NONE) {
                continue;
            }
            if (!copyVideo) {
                ret = prepare_encoder_video(decoder, encoder);
                if (ret < 0) {
                    goto end;
                }
            } else {
                prepare_copy(encoder->fmtCtx, &encoder->videoStream,
                             decoder->videoStream->codecpar);
            }
        } else if (decoder->fmtCtx->streams[i]->codecpar->codec_type ==
                   AVMEDIA_TYPE_AUDIO) {
            // skip audio streams
            if (encoder->fmtCtx->oformat->audio_codec == AV_CODEC_ID_NONE) {
                continue;
            }
            if (!copyAudio) {
                ret = prepare_encoder_audio(decoder, encoder);
                if (ret < 0) {
                    goto end;
                }
            } else {
                prepare_copy(encoder->fmtCtx, &encoder->audioStream,
                             decoder->audioStream->codecpar);
            }
        }
    }
    // binding
    ret = avio_open2(&encoder->fmtCtx->pb, encoder->filename, AVIO_FLAG_WRITE,
                     NULL, NULL);
    if (ret < 0) {
        print_error("Failed to open output file", ret);
        flag = false;
        goto end;
    }
    /* Write the stream header, if any. */
    ret = avformat_write_header(encoder->fmtCtx, NULL);
    if (ret < 0) {
        print_error("Failed to write header", ret);
        flag = false;
        goto end;
    }

    // read video data from multimedia files to write into destination file
    while (av_read_frame(decoder->fmtCtx, decoder->pkt) >= 0) {
        if (decoder->pkt->stream_index == decoder->videoIdx) {
            if (encoder->fmtCtx->oformat->video_codec == AV_CODEC_ID_NONE) {
                continue;
            }
            // Update progress based on video stream
            update_progress(decoder->pkt->pts, decoder->videoStream->time_base);

            if (!copyVideo) {
                transcode_video(decoder, encoder);
            } else {
                remux(decoder->pkt, encoder->fmtCtx, decoder->videoStream,
                      encoder->videoStream);
            }
        } else if (decoder->pkt->stream_index == decoder->audioIdx) {
            if (encoder->fmtCtx->oformat->audio_codec == AV_CODEC_ID_NONE) {
                continue;
            }
            // Update progress based on audio stream if no video stream
            if (decoder->videoIdx < 0) {
                update_progress(decoder->pkt->pts,
                                decoder->audioStream->time_base);
            }

            if (!copyAudio) {
                transcode_audio(decoder, encoder);
            } else {
                remux(decoder->pkt, encoder->fmtCtx, decoder->audioStream,
                      encoder->audioStream);
            }
        }
    }
    if (!copyVideo) {
        encoder->frame = NULL;
        // write the buffered frame
        encode_video(decoder->videoStream, encoder, NULL);
    }

    processParameter->set_process_number(1, 1);

    av_write_trailer(encoder->fmtCtx);

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

bool TranscoderFFmpeg::open_media(StreamContext *decoder,
                                  StreamContext *encoder) {
    int ret = -1;
    /* set the frameNumber to zero to avoid some bugs */
    frameNumber = 0;
    // open the multimedia file
    if ((ret = avformat_open_input(&decoder->fmtCtx, decoder->filename, NULL,
                                   NULL)) < 0) {
        print_error("Failed to open input file", ret);
        return false;
    }

    ret = avformat_find_stream_info(decoder->fmtCtx, NULL);
    if (ret < 0) {
        print_error("Failed to find stream info", ret);
        return false;
    }

    ret = avformat_alloc_output_context2(&encoder->fmtCtx, NULL, NULL,
                                         encoder->filename);
    if (!encoder->fmtCtx) {
        av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
        return false;
    }

    return true;
}

bool TranscoderFFmpeg::encode_video(AVStream *inStream, StreamContext *encoder,
                                    AVFrame *inputFrame) {
    int ret = -1;
    AVPacket *output_packet = av_packet_alloc();

    if (filter_graph) {
        /* push the decoded frame into the filtergraph */
        if (av_buffersrc_add_frame_flags(buffersrc_ctx, inputFrame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
            av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
            return false;
        }
        /* pull filtered frames from the filtergraph */
        while (1) {
            ret = av_buffersink_get_frame(buffersink_ctx, inputFrame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            if (ret < 0)
                goto end;
        }
    }

    if (encodeParameter->get_qscale() != -1 && inputFrame) {
        inputFrame->quality = encoder->videoCodecCtx->global_quality;
        inputFrame->pict_type = AV_PICTURE_TYPE_NONE;
    }
    // send frame to encoder
    ret = avcodec_send_frame(encoder->videoCodecCtx, inputFrame);
    if (ret < 0) {
        print_error("Failed to send frame to encoder", ret);
        goto end;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(encoder->videoCodecCtx, output_packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return true;
        } else if (ret < 0) {
            return false;
        }

        output_packet->stream_index = encoder->videoStream->index;
        output_packet->duration = encoder->videoStream->time_base.den /
                                  encoder->videoStream->time_base.num /
                                  inStream->avg_frame_rate.num *
                                  inStream->avg_frame_rate.den;

        av_packet_rescale_ts(output_packet, inStream->time_base,
                             encoder->videoStream->time_base);

        ret = av_interleaved_write_frame(encoder->fmtCtx, output_packet);
        if (ret < 0) {
            print_error("Failed to write packet", ret);
        }

        av_packet_unref(output_packet);
    }

end:
    return true;
}

bool TranscoderFFmpeg::encode_audio(AVStream *in_stream, StreamContext *encoder,
                                    AVFrame *input_frame) {
    int ret = -1;
    AVPacket *output_packet = av_packet_alloc();
    // send frame to encoder
    ret = avcodec_send_frame(encoder->audioCodecCtx, input_frame);
    if (ret < 0) {
        print_error("Failed to send frame to encoder", ret);
        goto end;
    }
    while (ret >= 0) {
        ret = avcodec_receive_packet(encoder->audioCodecCtx, output_packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        } else if (ret < 0) {
            return -1;
        }
        output_packet->stream_index = encoder->audioStream->index;
        av_packet_rescale_ts(output_packet, in_stream->time_base,
                             encoder->audioStream->time_base);
        ret = av_interleaved_write_frame(encoder->fmtCtx, output_packet);
        if (ret < 0) {
            print_error("Failed to write packet", ret);
        }
        av_packet_unref(output_packet);
    }

end:
    return 0;
}

bool TranscoderFFmpeg::transcode_video(StreamContext *decoder,
                                       StreamContext *encoder) {
    int ret = -1;

    // send packet to decoder
    ret = avcodec_send_packet(decoder->videoCodecCtx, decoder->pkt);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to send packet to decoder!\n");
        goto end;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(decoder->videoCodecCtx, decoder->frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        } else if (ret < 0) {
            return -1;
        }

        encode_video(decoder->videoStream, encoder, decoder->frame);

        if (decoder->pkt) {
            av_packet_unref(decoder->pkt);
        }

        av_frame_unref(decoder->frame);
    }

end:
    return 0;
}

bool TranscoderFFmpeg::transcode_audio(StreamContext *decoder,
                                       StreamContext *encoder) {
    int ret = avcodec_send_packet(decoder->audioCodecCtx, decoder->pkt);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to send packet to decoder!\n");
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(decoder->audioCodecCtx, decoder->frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR,
                   "Failed to receive frame from decoder!\n");
            return ret;
        }

        encode_audio(decoder->audioStream, encoder, decoder->frame);

        if (decoder->pkt) {
            av_packet_unref(decoder->pkt);
        }
        av_frame_unref(decoder->frame);
    }
    return 0;
}

bool TranscoderFFmpeg::prepare_decoder(StreamContext *decoder) {
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
            // return -1;
        }
        // init decoder context
        decoder->videoCodecCtx = avcodec_alloc_context3(decoder->videoCodec);
        if (!decoder->videoCodec) {
            av_log(decoder->videoCodecCtx, AV_LOG_ERROR, "No memory!\n");
            // return -1;
        }
        avcodec_parameters_to_context(decoder->videoCodecCtx,
                                    decoder->videoStream->codecpar);
        // bind decoder and decoder context
        ret = avcodec_open2(decoder->videoCodecCtx, decoder->videoCodec, NULL);
        if (ret < 0) {
            print_error("Couldn't open the codec", ret);
            return false;
        }
    }

    if (decoder->audioIdx != OC_INVALID_STREAM_IDX) {
        decoder->audioCodec =
            avcodec_find_decoder(decoder->audioStream->codecpar->codec_id);
        if (!decoder->audioCodec) {
            av_log(NULL, AV_LOG_ERROR, "Couldn't find codec: %s \n",
                    avcodec_get_name(decoder->audioStream->codecpar->codec_id));
        }
        decoder->audioCodecCtx = avcodec_alloc_context3(decoder->audioCodec);
        if (!decoder->audioCodec) {
            av_log(decoder->audioCodecCtx, AV_LOG_ERROR, "No Memory!");
        }
        avcodec_parameters_to_context(decoder->audioCodecCtx,
                                    decoder->audioStream->codecpar);


        ret = avcodec_open2(decoder->audioCodecCtx, decoder->audioCodec, NULL);
        if (ret < 0) {
            print_error("Couldn't open the codec", ret);
            return false;
        }
    }

    // create AVFrame
    decoder->frame = av_frame_alloc();
    if (!decoder->frame) {
        av_log(NULL, AV_LOG_ERROR, "No Memory!\n");
        return false;
    }

    // create AVPacket
    decoder->pkt = av_packet_alloc();
    if (!decoder->pkt) {
        av_log(NULL, AV_LOG_ERROR, "NO Memory!\n");
        return false;
    }

    return true;
}

bool TranscoderFFmpeg::prepare_encoder_video(StreamContext *decoder,
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
        return false;
    }

    // init codec context
    encoder->videoCodecCtx = avcodec_alloc_context3(encoder->videoCodec);
    if (!encoder->videoCodecCtx) {
        av_log(NULL, AV_LOG_ERROR, "No memory!\n");
        return false;
    }

    av_opt_set(encoder->videoCodecCtx->priv_data, "preset", "medium", 0);
    if (encoder->videoCodecCtx->codec_id == AV_CODEC_ID_H264)
        av_opt_set(encoder->videoCodecCtx->priv_data, "x264-params",
                   "keyint=60:min-keyint=60:scenecut=0:force-cfr=1", 0);
    else if (encoder->videoCodecCtx->codec_id == AV_CODEC_ID_HEVC)
        av_opt_set(encoder->videoCodecCtx->priv_data, "x265-params",
                   "keyint=60:min-keyint=60:scenecut=0", 0);

    if (decoder->videoCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO) {
        uint16_t width = encodeParameter->get_width();
        uint16_t height = encodeParameter->get_height();
        std::string pixelFormat = encodeParameter->get_pixel_format();
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
            encoder->videoCodecCtx->bit_rate = decoder->videoCodecCtx->bit_rate;
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
        encoder->videoCodecCtx->time_base = av_make_q(1, 60);
        encoder->videoCodecCtx->framerate = av_make_q(60, 1);
        int qscale = encodeParameter->get_qscale();
        if (qscale != -1) {
            encoder->videoCodecCtx->flags |= AV_CODEC_FLAG_QSCALE;
            encoder->videoCodecCtx->global_quality = qscale * FF_QP2LAMBDA;
        }
    }

    // bind codec and codec context
    ret = avcodec_open2(encoder->videoCodecCtx, encoder->videoCodec, NULL);
    if (ret < 0) {
        print_error("Couldn't open the codec", ret);
        return false;
    }

    encoder->videoStream = avformat_new_stream(encoder->fmtCtx, NULL);
    if (!encoder->videoStream) {
        av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
        return false;
    }
    encoder->videoStream->r_frame_rate =
        av_make_q(60, 1); // For setting real frame rate
    encoder->videoStream->avg_frame_rate =
        av_make_q(60, 1); // For setting average frame rate
    // the input file's time_base is wrong
    encoder->videoStream->time_base = encoder->videoCodecCtx->time_base;

    ret = avcodec_parameters_from_context(encoder->videoStream->codecpar,
                                          encoder->videoCodecCtx);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR,
               "Failed to copy encoder parameters to output stream #\n");
        return false;
    }

    // oFmtCtx->oformat = av_guess_format(NULL, dst, NULL);
    // if(!oFmtCtx->oformat)
    // {
    //     av_log(NULL, AV_LOG_ERROR, "No Memory!\n");
    // }

    return true;
}

bool TranscoderFFmpeg::prepare_encoder_audio(StreamContext *decoder,
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
        return -1;
    }
    // init codec context
    encoder->audioCodecCtx = avcodec_alloc_context3(encoder->audioCodec);
    if (!encoder->audioCodecCtx) {
        av_log(NULL, AV_LOG_ERROR, "No memory!\n");
        return -1;
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
    ret = avcodec_open2(encoder->audioCodecCtx, encoder->audioCodec, NULL);
    if (ret < 0) {
        print_error("Couldn't open the codec", ret);
        return false;
    }
    encoder->audioStream = avformat_new_stream(encoder->fmtCtx, NULL);
    if (!encoder->audioStream) {
        av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
        return -1;
    }
    encoder->audioStream->time_base = encoder->audioCodecCtx->time_base;
    ret = avcodec_parameters_from_context(encoder->audioStream->codecpar,
                                          encoder->audioCodecCtx);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR,
               "Failed to copy encoder parameters to output stream #\n");
        return -1;
    }
    return true;
}

bool TranscoderFFmpeg::prepare_copy(AVFormatContext *avCtx, AVStream **stream,
                                    AVCodecParameters *codecParam) {
    *stream = avformat_new_stream(avCtx, NULL);
    avcodec_parameters_copy((*stream)->codecpar, codecParam);

    // the "mp4a" tag in MP4 is incompatible with MKV.
    // so set the codec_tag of the audio stream to 0 to avoid compatibility
    // issues. we will improve this method in the future.
    if (codecParam->codec_type == AVMEDIA_TYPE_AUDIO) {
        (*stream)->codecpar->codec_tag = 0;
    }
    return true;
}

bool TranscoderFFmpeg::remux(AVPacket *pkt, AVFormatContext *avCtx,
                             AVStream *inStream, AVStream *outStream) {
    // associate the avpacket with the target output avstream
    pkt->stream_index = outStream->index;
    av_packet_rescale_ts(pkt, inStream->time_base, outStream->time_base);
    if (av_interleaved_write_frame(avCtx, pkt) < 0) {
        av_log(NULL, AV_LOG_ERROR, "write frame error!\n");
        return false;
    }
    return true;
}

TranscoderFFmpeg::~TranscoderFFmpeg() {}
