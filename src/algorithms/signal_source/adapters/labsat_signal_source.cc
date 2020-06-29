/*!
 * \file labsat_signal_source.cc
 * \brief Labsat 2 and 3 front-end signal sampler driver
 * \author Javier Arribas, jarribas(at)cttc.es
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * -------------------------------------------------------------------------
 */

#include "labsat_signal_source.h"
#include "configuration_interface.h"
#include "labsat23_source.h"
#include <glog/logging.h>
#include <cstdint>
#include <utility>


LabsatSignalSource::LabsatSignalSource(const ConfigurationInterface* configuration,
    const std::string& role, unsigned int in_stream, unsigned int out_stream, Concurrent_Queue<pmt::pmt_t>* queue) : role_(role), in_stream_(in_stream), out_stream_(out_stream)
{
    std::string default_item_type = "gr_complex";
    std::string default_dump_file = "./labsat_output.dat";
    item_type_ = configuration->property(role + ".item_type", default_item_type);
    dump_ = configuration->property(role + ".dump", false);
    dump_filename_ = configuration->property(role + ".dump_filename", default_dump_file);

    int channel_selector = configuration->property(role + ".selected_channel", 1);

    std::string default_filename = "./example_capture.LS3";
    filename_ = configuration->property(role + ".filename", default_filename);

    if (item_type_ == "gr_complex")
        {
            item_size_ = sizeof(gr_complex);
            labsat23_source_ = labsat23_make_source_sptr(filename_.c_str(), channel_selector, queue);
            DLOG(INFO) << "Item size " << item_size_;
            DLOG(INFO) << "labsat23_source_(" << labsat23_source_->unique_id() << ")";
        }
    else
        {
            LOG(WARNING) << item_type_ << " unrecognized item type for LabSat source";
            item_size_ = sizeof(int16_t);
        }
    if (dump_)
        {
            DLOG(INFO) << "Dumping output into file " << dump_filename_;
            file_sink_ = gr::blocks::file_sink::make(item_size_, dump_filename_.c_str());
        }
    if (dump_)
        {
            DLOG(INFO) << "file_sink(" << file_sink_->unique_id() << ")";
        }
    if (in_stream_ > 0)
        {
            LOG(ERROR) << "A signal source does not have an input stream";
        }
    if (out_stream_ > 1)
        {
            LOG(ERROR) << "This implementation only supports one output stream";
        }
}


void LabsatSignalSource::connect(gr::top_block_sptr top_block)
{
    if (dump_)
        {
            top_block->connect(labsat23_source_, 0, file_sink_, 0);
            DLOG(INFO) << "connected labsat23_source_ to file sink";
        }
    else
        {
            DLOG(INFO) << "nothing to connect internally";
        }
}


void LabsatSignalSource::disconnect(gr::top_block_sptr top_block)
{
    if (dump_)
        {
            top_block->disconnect(labsat23_source_, 0, file_sink_, 0);
        }
}


gr::basic_block_sptr LabsatSignalSource::get_left_block()
{
    LOG(WARNING) << "Left block of a signal source should not be retrieved";
    return gr::block_sptr();
}


gr::basic_block_sptr LabsatSignalSource::get_right_block()
{
    return labsat23_source_;
}
