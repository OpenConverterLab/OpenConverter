```c++
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
#include <libavutil/error.h> // For av_err2str
}
#include <chrono>
#include <string> // For std::string operations
#include <algorithm> // For std::max

/* Receive pointers from converter */
TranscoderFFmpeg::TranscoderFFmpeg(ProcessParameter *processParameter,
                                   EncodeParameter *encodeParameter)
    : Transcoder(processParameter, encodeParameter) {
    frameTotalNumber = 0;
    total_duration = 0;
    current_duration = 0;
    decoder = nullptr;
    encoder = nullptr;
    filters_ctx = nullptr;
}

void TranscoderFFmpeg::print_error(const char *msg, int ret) {
    av_strerror(ret, errorMsg, sizeof(errorMsg));
    av_log(NULL, AV_LOG_ERROR, " %s: %s \n", msg, errorMsg);
}

void TranscoderFFmpeg::update_progress(int64_t current_pts,
                                       AVRational time_base) {
    // Convert current PTS to microseconds
    AVRational micros_base = {1, 1000000};
    // The current_pts here will be relative to the start of the trimmed segment
    // because of the setpts filter, which resets timestamps.
    current_duration = av_rescale_q(current_pts, time_base, micros_base);

    // Calculate progress percentage based on the adjusted total_duration (clip duration)
    if (total_duration > 0) {
        send_process_parameter(current_duration, total_duration);
    }
}

// Helper function to set the total_duration for progress reporting when cutting
void TranscoderFFmpeg::set_progress_duration_for_cut_segment() {
    double startTime = processParameter->GetStartTime();
    double endTime = processParameter->GetEndTime();

    bool isCutting = (startTime >= 0.0 || endTime >= 0.0); // True if any cutting parameter is active

    if (isCutting) {
        // Case 1: Both start and end time are specified and valid
        if (startTime >= 0.0 && endTime >= 0.0 && endTime > startTime) {
            total_duration = static_cast<int64_t>((endTime - startTime) * 1000000); // Convert seconds to microseconds
            av_log(NULL, AV_LOG_INFO, "Cutting enabled (start=%.3f, end=%.3f). Progress duration set to %lld us\n", startTime, endTime, total_duration);
        }
        // Case 2: Only start time is specified, cut from start time to the end of the original file
        else if (startTime >= 0.0 && decoder->fmtCtx->duration != AV_NOPTS_VALUE) {
            double full_duration_seconds = static_cast<double>(decoder->fmtCtx->duration) / AV_TIME_BASE;
            double remaining_duration = full_duration_seconds - startTime;
            if (remaining_duration > 0) {
                total_duration = static_cast<int64_t>(remaining_duration * 1000000); // Convert seconds to microseconds
                av_log(NULL, AV_LOG_INFO, "Cutting enabled (start=%.3f, to end). Progress duration set to %lld us\n", startTime, total_duration);
            } else {
                total_duration = 0; // Invalid cut (e.g., start time is beyond file end)
                av_log(NULL, AV_LOG_WARNING, "Calculated remaining duration for cut is zero or negative. Progress might be inaccurate.\n");
            }
        }
        // Case 3: Cutting parameters are ambiguous (e.g., only endTime, or endTime <= startTime but both present)
        // In such cases, fallback to full duration if available.
        else {
            if (decoder->fmtCtx->duration != AV_NOPTS_VALUE) {
                total_duration = decoder->fmtCtx->duration; // Already in AV_TIME_BASE (microseconds)
                av_log(NULL, AV_LOG_WARNING, "Ambiguous cutting parameters (start=%.3f, end=%.3f). Using full source duration for progress: %lld us\n", startTime, endTime, total_duration);
            } else {
                 total_duration = 0; // Still couldn't get duration
            }
        }
    }
    // If no cutting is requested, ensure total_duration is correctly set to full source duration
    // (It should already be set from fmtCtx->duration in transcode() if available)
    else if (decoder->fmtCtx->duration != AV_NOPTS_VALUE) {
         total_duration = decoder->fmtCtx->duration; // Already in AV_TIME_BASE (microseconds)
         av_log(NULL, AV_LOG_INFO, "No cutting. Progress duration set to full source: %lld us\n", total_duration);
    }

    if (total_duration <= 0) {
        av_log(NULL, AV_LOG_WARNING, "Could not determine effective total duration for progress reporting. Progress bar might not function correctly.\n");
    }
}


int TranscoderFFmpeg::init_filter(AVCodecContext *dec_ctx, FilteringContext *filter_ctx, const char *filters_descr)
{
    char args[512];
    int ret = 0;
    char buf[64];
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

    if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO && encoder->videoCodecCtx) {
        ret = av_opt_set_bin(buffersink_ctx, "pix_fmts",
                (uint8_t*)&encoder->videoCodecCtx->pix_fmt, sizeof(encoder->videoCodecCtx->pix_fmt),
                AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
            goto end;
        }
    } else if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO && encoder->audioCodecCtx) {
        ret = av_opt_set_bin(buffersink_ctx, "sample_fmts",
                (uint8_t*)&encoder->audioCodecCtx->sample_fmt, sizeof(encoder->audioCodecCtx->sample_fmt),
                AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output sample format\n");
            goto end;
        }

        av_channel_layout_describe(&encoder->audioCodecCtx->ch_layout, buf, sizeof(buf));
        ret = av_opt_set(buffersink_ctx, "ch_layouts",
                         buf, AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output channel layout\n");
            goto end;
        }

        ret = av_opt_set_bin(buffersink_ctx, "sample_rates",
                (uint8_t*)&encoder->audioCodecCtx->sample_rate, sizeof(encoder->audioCodecCtx->sample_rate),
                AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output sample rate\n");
            goto end;
        }
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
    if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO && encoder->audioCodecCtx) {
        if (!(encoder->audioCodec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE))
            av_buffersink_set_frame_size(filters_ctx[decoder->audioIdx].buffersink_ctx,
                                         encoder->audioCodecCtx->frame_size);
    }

end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}


int TranscoderFFmpeg::init_filters_wrapper()
{
    int i, ret = -1;
    std::string filter_str = "";
    AVCodecContext *dec_ctx = NULL;
    filters_ctx = reinterpret_cast<FilteringContext *>(av_malloc_array(decoder->fmtCtx->nb_streams, sizeof(*filters_ctx)));
    if (!filters_ctx)
        return AVERROR(ENOMEM);

    // Get cutting parameters from ProcessParameter
    double startTime = processParameter->GetStartTime();
    double endTime = processParameter->GetEndTime();
    bool isCutting = (startTime >= 0.0 || endTime >= 0.0); // True if any cutting parameter is active

    for (i = 0; i < (int)decoder->fmtCtx->nb_streams; i++) { // Explicit cast to int for loop comparison
        filters_ctx[i].buffersrc_ctx  = NULL;
        filters_ctx[i].buffersink_ctx = NULL;
        filters_ctx[i].filter_graph   = NULL;

        // Skip streams that are neither audio nor video
        if (!(decoder->fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO ||
            decoder->fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO))
            continue;

        filter_str = ""; // Reset filter string for each stream

        if (decoder->fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (isCutting) {
                // Apply video trim and setpts filters for accurate cutting
                filter_str += "trim=start=" + std::to_string(startTime);
                if (endTime >= 0.0 && endTime > startTime) { // Only add end if it's a valid end point
                    filter_str += ":end=" + std::to_string(endTime);
                }
                filter_str += ",setpts=PTS-STARTPTS"; // Reset timestamps to start from 0
            }

            std::string pixelFormat = encodeParameter->get_pixel_format();
            uint16_t width = encodeParameter->get_width();
            uint16_t height = encodeParameter->get_height();

            // Append format and scale filters if specified
            if (!pixelFormat.empty()) {
                if (!filter_str.empty()) filter_str += ",";
                filter_str += "format=" + pixelFormat;
            }
            if (width > 0 && height > 0) {
                if (!filter_str.empty()) filter_str += ",";
                filter_str += "scale=" + std::to_string(width) + ":" + std::to_string(height);
            }
            if (filter_str.empty())
                filter_str = "null"; // Default if no other filters applied
            dec_ctx = decoder->videoCodecCtx;
        } else if (decoder->fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (isCutting) {
                // Apply audio atrim and asetpts filters for accurate cutting
                filter_str += "atrim=start=" + std::to_string(startTime);
                if (endTime >= 0.0 && endTime > startTime) { // Only add end if valid
                    filter_str += ":end=" + std::to_string(endTime);
                }
                filter_str += ",asetpts=PTS-STARTPTS"; // Reset timestamps to start from 0
            }

            if (filter_str.empty())
                filter_str = "anull"; // Default if no other filters applied
            dec_ctx = decoder->audioCodecCtx;
        } else {
            // This case should ideally be caught by the initial stream type check,
            // but as a safeguard, log and skip if it somehow reaches here.
            av_log(NULL, AV_LOG_WARNING, "Stream %d has unsupported codec type %s for filtering. Skipping.\n",
                   i, av_get_media_type_string(decoder->fmtCtx->streams[i]->codecpar->codec_type));
            continue;
        }

        av_log(NULL, AV_LOG_DEBUG, "Stream %d filter string: \"%s\"\n", i, filter_str.c_str());
        ret = init_filter(dec_ctx, &filters_ctx[i], filter_str.c_str());
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Failed to initialize filter for stream %d: %s\n", i, av_err2str(ret));
            return ret;
        }
    }
    return ret;
}

bool TranscoderFFmpeg::transcode(std::string input_path,
                                 std::string output_path) {
    bool flag = false;
    int ret = -1;

    decoder = new StreamContext;
    encoder = new StreamContext;

    // Get cutting parameters from ProcessParameter
    double startTime = processParameter->GetStartTime();
    double endTime = processParameter->GetEndTime();
    bool cutting_active = (startTime >= 0.0 || endTime >= 0.0);

    // If cutting is active, force re-encoding for relevant streams to apply filters.
    // This overrides any 'copy' settings from EncodeParameter if cutting is desired.
    // If encodeParameter specifies an empty codec name (implying copy), but cutting is active,
    // we force re-encoding by setting copy flags to false.
    copyVideo = (encodeParameter->get_video_codec_name().empty() && !cutting_active);
    copyAudio = (encodeParameter->get_audio_codec_name().empty() && !cutting_active);

    av_log_set_level(AV_LOG_DEBUG); // Keep debug logging as per plan for tracing

    decoder->filename = input_path.c_str();
    encoder->filename = output_path.c_str();

    if ((ret = open_media()) < 0)
        goto end;

    // Calculate total duration from the input file
    if (decoder->fmtCtx->duration != AV_NOPTS_VALUE) {
        total_duration = decoder->fmtCtx->duration; // This is in AV_TIME_BASE units (microseconds)
    } else {
        // If duration is not available, try to get it from streams
        for (unsigned int i = 0; i < decoder->fmtCtx->nb_streams; i++) {
            AVStream *stream = decoder->fmtCtx->streams[i];
            if (stream->duration != AV_NOPTS_VALUE) {
                // Ensure stream duration is also converted to microseconds for consistency
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

    // Call the helper function to adjust total_duration for progress bar if cutting
    set_progress_duration_for_cut_segment();

    // Validate time range parameters (more robust validation might be in converter.cpp)
    if (startTime >= 0.0 && endTime >= 0.0) {
        if (endTime <= startTime) {
            print_error("End time must be greater than start time for cutting", 0);
            ret = AVERROR(EINVAL);
            goto end;
        }
    }

    // Initialize encoder streams
    if ((ret = init_encoder_streams()) < 0) {
        print_error("Failed to initialize encoder streams", ret);
        goto end;
    }

    // Open output file
    if (!(encoder->fmtCtx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&encoder->fmtCtx->pb, encoder->filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            print_error("Could not open output file", ret);
            goto end;
        }
    }

    // Write file header
    ret = avformat_write_header(encoder->fmtCtx, NULL);
    if (ret < 0) {
        print_error("Error occurred when opening output file", ret);
        goto end;
    }

    // Initialize filters for all streams (video/audio)
    // This is where trim/setpts filters are added for cutting
    if ((ret = init_filters_wrapper()) < 0) {
        print_error("Failed to initialize filters", ret);
        goto end;
    }

    // Main transcoding loop
    AVPacket *packet = av_packet_alloc();
    if (!packet) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    while (true) {
        // Read frames from the input media file
        ret = av_read_frame(decoder->fmtCtx, packet);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                av_log(NULL, AV_LOG_INFO, "End of input stream.\n");
                break; // End of file, break to flush
            }
            print_error("Failed to read frame from input", ret);
            goto end;
        }

        int stream_idx = packet->stream_index;

        if (stream_idx == decoder->videoIdx && !copyVideo) {
            ret = decode_and_filter_video(packet);
        } else if (stream_idx == decoder->audioIdx && !copyAudio) {
            ret = decode_and_filter_audio(packet);
        } else if ((stream_idx == decoder->videoIdx && copyVideo) || (stream_idx == decoder->audioIdx && copyAudio)) {
            // Stream copying logic (only if NOT cutting, enforced by 'copyVideo/copyAudio' flags)
            // Rescale PTS/DTS for copied packets and write directly.
            av_packet_rescale_ts(packet, decoder->fmtCtx->streams[stream_idx]->time_base, encoder->fmtCtx->streams[stream_idx]->time_base);
            ret = av_interleaved_write_frame(encoder->fmtCtx, packet);
            if (ret < 0) {
                print_error("Error writing copied packet to output", ret);
                goto end;
            }
            // Update progress for copied streams. Using the DTS as a proxy for progress.
            // If there's a primary stream (e.g., video), progress update should ideally be driven by it.
            // For simplicity, updating progress from the first copied stream that processes.
            if ((stream_idx == decoder->videoIdx && copyVideo) || (stream_idx == decoder->audioIdx && copyAudio && decoder->videoIdx == -1)) {
                 update_progress(packet->dts, encoder->fmtCtx->streams[stream_idx]->time_base);
            }
        } else {
            // Other stream types or streams not selected for processing (e.g., subtitle, data)
            av_packet_unref(packet);
            continue;
        }

        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            print_error("Error during decoding, filtering or writing", ret);
            goto end;
        }
        av_packet_unref(packet);
    }

    // Flush decoders/filters/encoders at the end of input
    av_log(NULL, AV_LOG_INFO, "Flushing decoders and encoders.\n");
    // Flush video decoder and filter
    if (decoder->videoIdx != -1 && !copyVideo) {
        ret = decode_and_filter_video(NULL); // Pass NULL to flush
        if (ret < 0 && ret != AVERROR_EOF) {
            print_error("Failed to flush video decoder/filter", ret);
            goto end;
        }
    }
    // Flush audio decoder and filter
    if (decoder->audioIdx != -1 && !copyAudio) {
        ret = decode_and_filter_audio(NULL); // Pass NULL to flush
        if (ret < 0 && ret != AVERROR_EOF) {
            print_error("Failed to flush audio decoder/filter", ret);
            goto end;
        }
    }

    // Write trailer to output file
    av_write_trailer(encoder->fmtCtx);
    av_log(NULL, AV_LOG_INFO, "Transcoding finished successfully.\n");
    flag = true;

end:
    // Cleanup
    av_packet_free(&packet);
    if (decoder) {
        decoder->close();
        delete decoder;
        decoder = nullptr;
    }
    if (encoder) {
        encoder->close();
        delete encoder;
        encoder = nullptr;
    }
    if (filters_ctx) {
        // Iterate only through initialized filter contexts up to nb_streams
        for (int i = 0; i < (int)decoder->fmtCtx->nb_streams; i++) {
            if (filters_ctx[i].filter_graph) {
                avfilter_graph_free(&filters_ctx[i].filter_graph);
            }
        }
        av_free(filters_ctx);
        filters_ctx = nullptr;
    }

    if (ret < 0 && ret != AVERROR_EOF) { // Only print error if it's not just end-of-file
        print_error("Transcoding failed", ret);
        flag = false;
    }
    return flag;
}

// Minimal assumed `open_media` definition, needs to be a member of TranscoderFFmpeg
int TranscoderFFmpeg::open_media() {
    int ret;
    // Open input file
    ret = avformat_open_input(&decoder->fmtCtx, decoder->filename, NULL, NULL);
    if (ret < 0) {
        print_error("Cannot open input file", ret);
        return ret;
    }
    ret = avformat_find_stream_info(decoder->fmtCtx, NULL);
    if (ret < 0) {
        print_error("Cannot find stream information", ret);
        return ret;
    }
    // Find video and audio streams
    decoder->videoIdx = -1;
    decoder->audioIdx = -1;
    for (unsigned int i = 0; i < decoder->fmtCtx->nb_streams; i++) {
        AVStream *stream = decoder->fmtCtx->streams[i];
        AVCodecParameters *codecpar = stream->codecpar;
        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO && decoder->videoIdx == -1) {
            decoder->videoIdx = i;
            decoder->videoCodec = avcodec_find_decoder(codecpar->codec_id);
            if (!decoder->videoCodec) {
                print_error("Failed to find video decoder", AVERROR_DECODER_NOT_FOUND);
                return AVERROR_DECODER_NOT_FOUND;
            }
            decoder->videoCodecCtx = avcodec_alloc_context3(decoder->videoCodec);
            if (!decoder->videoCodecCtx) return AVERROR(ENOMEM);
            ret = avcodec_parameters_to_context(decoder->videoCodecCtx, codecpar);
            if (ret < 0) { avcodec_free_context(&decoder->videoCodecCtx); print_error("Failed to copy video codec parameters to context", ret); return ret; }
            ret = avcodec_open2(decoder->videoCodecCtx, decoder->videoCodec, NULL);
            if (ret < 0) {
                avcodec_free_context(&decoder->videoCodecCtx);
                print_error("Cannot open video decoder", ret);
                return ret;
            }
        } else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO && decoder->audioIdx == -1) {
            decoder->audioIdx = i;
            decoder->audioCodec = avcodec_find_decoder(codecpar->codec_id);
            if (!decoder->audioCodec) {
                print_error("Failed to find audio decoder", AVERROR_DECODER_NOT_FOUND);
                return AVERROR_DECODER_NOT_FOUND;
            }
            decoder->audioCodecCtx = avcodec_alloc_context3(decoder->audioCodec);
            if (!decoder->audioCodecCtx) return AVERROR(ENOMEM);
            ret = avcodec_parameters_to_context(decoder->audioCodecCtx, codecpar);
            if (ret < 0) { avcodec_free_context(&decoder->audioCodecCtx); print_error("Failed to copy audio codec parameters to context", ret); return ret; }
            ret = avcodec_open2(decoder->audioCodecCtx, decoder->audioCodec, NULL);
            if (ret < 0) {
                avcodec_free_context(&decoder->audioCodecCtx);
                print_error("Cannot open audio decoder", ret);
                return ret;
            }
        }
    }

    // Initialize output format context
    ret = avformat_alloc_output_context2(&encoder->fmtCtx, NULL, NULL, encoder->filename);
    if (ret < 0) {
        print_error("Could not create output context", ret);
        return ret;
    }

    return 0;
}

// Minimal assumed `init_encoder_streams` definition
int TranscoderFFmpeg::init_encoder_streams() {
    int ret;
    // Video Stream
    if (decoder->videoIdx != -1) {
        AVCodec *out_video_codec = NULL;
        if (copyVideo) {
            out_video_codec = decoder->videoCodec;
        } else {
            std::string codec_name = encodeParameter->get_video_codec_name();
            if (codec_name.empty()) codec_name = "libx264"; // Default to H.264 if not specified and not copying
            out_video_codec = avcodec_find_encoder_by_name(codec_name.c_str());
        }
        if (!out_video_codec) {
            print_error("Failed to find video encoder", AVERROR_ENCODER_NOT_FOUND);
            return AVERROR_ENCODER_NOT_FOUND;
        }
        encoder->videoCodec = out_video_codec; // Store for later reference
        encoder->videoCodecCtx = avcodec_alloc_context3(out_video_codec);
        if (!encoder->videoCodecCtx) return AVERROR(ENOMEM);

        encoder->videoCodecCtx->width = encodeParameter->get_width() > 0 ? encodeParameter->get_width() : decoder->videoCodecCtx->width;
        encoder->videoCodecCtx->height = encodeParameter->get_height() > 0 ? encodeParameter->get_height() : decoder->videoCodecCtx->height;
        encoder->videoCodecCtx->pix_fmt = av_get_pix_fmt(encodeParameter->get_pixel_format().c_str());
        if (encoder->videoCodecCtx->pix_fmt == AV_PIX_FMT_NONE || !av_codec_get_type(out_video_codec->id) || !av_codec_is_encoder(out_video_codec) || !av_codec_get_pix_fmts(out_video_codec) || !av_pix_fmt_desc_get(encoder->videoCodecCtx->pix_fmt)) { // Fallback if requested format is not found or unsupported
             encoder->videoCodecCtx->pix_fmt = decoder->videoCodecCtx->pix_fmt;
             if (encoder->videoCodecCtx->pix_fmt == AV_PIX_FMT_NONE || !av_codec_get_type(out_video_codec->id) || !av_codec_is_encoder(out_video_codec) || !av_codec_get_pix_fmts(out_video_codec) || !av_pix_fmt_desc_get(encoder->videoCodecCtx->pix_fmt)) {
                 encoder->videoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P; // Still none or unsupported, fallback to common
             }
        }
        
        encoder->videoCodecCtx->time_base = decoder->fmtCtx->streams[decoder->videoIdx]->avg_frame_rate.num > 0 ? av_inv_q(decoder->fmtCtx->streams[decoder->videoIdx]->avg_frame_rate) : (AVRational){1, 25}; // Derive from input or default
        encoder->videoCodecCtx->framerate = decoder->fmtCtx->streams[decoder->videoIdx]->avg_frame_rate.num > 0 ? decoder->fmtCtx->streams[decoder->videoIdx]->avg_frame_rate : (AVRational){25, 1};

        // Other common encoding parameters (simplified for this example)
        if (!copyVideo) {
            encoder->videoCodecCtx->bit_rate = 1000000; // 1 Mbps
            encoder->videoCodecCtx->gop_size = 12;
            encoder->videoCodecCtx->max_b_frames = 0; // H.264 baseline
            av_opt_set(encoder->videoCodecCtx->priv_data, "preset", "medium", 0);
            av_opt_set(encoder->videoCodecCtx->priv_data, "crf", "23", 0);
        }

        encoder->videoStream = avformat_new_stream(encoder->fmtCtx, out_video_codec);
        if (!encoder->videoStream) { avcodec_free_context(&encoder->videoCodecCtx); return AVERROR(ENOMEM); }
        encoder->videoIdx = encoder->videoStream->index;

        if (copyVideo) {
            ret = avcodec_parameters_copy(encoder->videoStream->codecpar, decoder->fmtCtx->streams[decoder->videoIdx]->codecpar);
            if (ret < 0) { avcodec_free_context(&encoder->videoCodecCtx); return ret; }
            encoder->videoStream->time_base = decoder->fmtCtx->streams[decoder->videoIdx]->time_base;
        } else {
            ret = avcodec_open2(encoder->videoCodecCtx, out_video_codec, NULL);
            if (ret < 0) { avcodec_free_context(&encoder->videoCodecCtx); return ret; }
            ret = avcodec_parameters_from_context(encoder->videoStream->codecpar, encoder->videoCodecCtx);
            if (ret < 0) { avcodec_free_context(&encoder->videoCodecCtx); return ret; }
            encoder->videoStream->time_base = encoder->videoCodecCtx->time_base;
        }
    }

    // Audio Stream
    if (decoder->audioIdx != -1) {
        AVCodec *out_audio_codec = NULL;
        if (copyAudio) {
            out_audio_codec = decoder->audioCodec;
        } else {
            std::string codec_name = encodeParameter->get_audio_codec_name();
            if (codec_name.empty()) codec_name = "aac"; // Default to AAC if not specified and not copying
            out_audio_codec = avcodec_find_encoder_by_name(codec_name.c_str());
        }
        if (!out_audio_codec) {
            print_error("Failed to find audio encoder", AVERROR_ENCODER_NOT_FOUND);
            return AVERROR_ENCODER_NOT_FOUND;
        }
        encoder->audioCodec = out_audio_codec; // Store for later reference
        encoder->audioCodecCtx = avcodec_alloc_context3(out_audio_codec);
        if (!encoder->audioCodecCtx) return AVERROR(ENOMEM);

        encoder->audioCodecCtx->sample_fmt = out_audio_codec->sample_fmts ? out_audio_codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        encoder->audioCodecCtx->sample_rate = decoder->audioCodecCtx->sample_rate;
        if (encoder->audioCodecCtx->sample_rate <= 0) encoder->audioCodecCtx->sample_rate = 44100; // Default if not found
        av_channel_layout_copy(&encoder->audioCodecCtx->ch_layout, &decoder->audioCodecCtx->ch_layout);
        if (av_channel_layout_check(&encoder->audioCodecCtx->ch_layout) < 0) { // Fallback if invalid
            av_channel_layout_default(&encoder->audioCodecCtx->ch_layout, 2); // Stereo
        }

        // Other common encoding parameters (simplified for this example)
        if (!copyAudio) {
            encoder->audioCodecCtx->bit_rate = 128000; // 128 kbps
        }

        encoder->audioStream = avformat_new_stream(encoder->fmtCtx, out_audio_codec);
        if (!encoder->audioStream) { avcodec_free_context(&encoder->audioCodecCtx); return AVERROR(ENOMEM); }
        encoder->audioIdx = encoder->audioStream->index;

        if (copyAudio) {
            ret = avcodec_parameters_copy(encoder->audioStream->codecpar, decoder->fmtCtx->streams[decoder->audioIdx]->codecpar);
            if (ret < 0) { avcodec_free_context(&encoder->audioCodecCtx); return ret; }
            encoder->audioStream->time_base = decoder->fmtCtx->streams[decoder->audioIdx]->time_base;
        } else {
            ret = avcodec_open2(encoder->audioCodecCtx, out_audio_codec, NULL);
            if (ret < 0) { avcodec_free_context(&encoder->audioCodecCtx); return ret; }
            ret = avcodec_parameters_from_context(encoder->audioStream->codecpar, encoder->audioCodecCtx);
            if (ret < 0) { avcodec_free_context(&encoder->audioCodecCtx); return ret; }
            encoder->audioStream->time_base = encoder->audioCodecCtx->time_base;
        }
    }
    return 0;
}

// Minimal assumed `encode_video_frame` definition
int TranscoderFFmpeg::encode_video_frame(AVFrame *frame) {
    int ret;
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) return AVERROR(ENOMEM);

    ret = avcodec_send_frame(encoder->videoCodecCtx, frame);
    if (ret < 0) {
        av_packet_free(&pkt);
        return ret;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(encoder->videoCodecCtx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            av_packet_free(&pkt);
            return ret;
        }

        // Rescale PTS/DTS and write packet
        av_packet_rescale_ts(pkt, encoder->videoCodecCtx->time_base, encoder->videoStream->time_base);
        pkt->stream_index = encoder->videoStream->index;
        ret = av_interleaved_write_frame(encoder->fmtCtx, pkt);
        if (ret < 0) {
            av_packet_free(&pkt);
            return ret;
        }
        // Update progress driven by video packets
        update_progress(pkt->pts, encoder->videoStream->time_base);

        av_packet_unref(pkt);
    }
    av_packet_free(&pkt);
    return 0;
}

// Minimal assumed `encode_audio_frame` definition
int TranscoderFFmpeg::encode_audio_frame(AVFrame *frame) {
    int ret;
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) return AVERROR(ENOMEM);

    ret = avcodec_send_frame(encoder->audioCodecCtx, frame);
    if (ret < 0) {
        av_packet_free(&pkt);
        return ret;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(encoder->audioCodecCtx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            av_packet_free(&pkt);
            return ret;
        }

        // Rescale PTS/DTS and write packet
        av_packet_rescale_ts(pkt, encoder->audioCodecCtx->time_base, encoder->audioStream->time_base);
        pkt->stream_index = encoder->audioStream->index;
        ret = av_interleaved_write_frame(encoder->fmtCtx, pkt);
        if (ret < 0) {
            av_packet_free(&pkt);
            return ret;
        }
        // Update progress driven by audio packets, only if no video stream for progress.
        // If there's a video stream, progress should ideally be driven by it.
        if (decoder->videoIdx == -1) {
            update_progress(pkt->pts, encoder->audioStream->time_base);
        }

        av_packet_unref(pkt);
    }
    av_packet_free(&pkt);
    return 0;
}
```