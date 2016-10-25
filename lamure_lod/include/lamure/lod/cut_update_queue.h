// Copyright (c) 2014 Bauhaus-Universitaet Weimar
// This Software is distributed under the Modified BSD License, see license.txt.
//
// Virtual Reality and Visualization Research Group 
// Faculty of Media, Bauhaus-Universitaet Weimar
// http://www.uni-weimar.de/medien/vr

#ifndef LAMURE_LOD_CUT_UPDATE_QUEUE_H_
#define LAMURE_LOD_CUT_UPDATE_QUEUE_H_

#include <lamure/types.h>
#include <lamure/assert.h>
#include <lamure/platform_lod.h>

#include <mutex>
#include <queue>

namespace lamure {
namespace lod {

class LOD_DLL cut_update_queue
{
public:

    enum task_t
    {
        CUT_MASTER_TASK,
        CUT_ANALYSIS_TASK,
        CUT_UPDATE_TASK,
        CUT_INVALID_TASK
    };

    struct job
    {
        explicit job(
            task_t task,
            const view_t view_id,
            const model_t model_id)
            : task_(task),
            view_id_(view_id),
            model_id_(model_id) {};

        explicit job()
            : task_(task_t::CUT_INVALID_TASK),
            view_id_(invalid_view_t),
            model_id_(invalid_model_t) {};

        task_t            task_;
        view_t          view_id_;
        model_t         model_id_;
    };

                        cut_update_queue();
    virtual             ~cut_update_queue();

    void                push_job(const job& job);
    const job           pop_front_job();

    const size_t        num_jobs();


private:
    std::queue<job>     job_queue_;
    std::mutex          mutex_;

};


} } // namespace lamure


#endif // LAMURE_LOD_CUT_UPDATE_QUEUE_H_
