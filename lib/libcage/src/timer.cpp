/*
 * Copyright (c) 2009, Yuuki Takano (ytakanoster@gmail.com).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the writers nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "timer.hpp"

#include <iostream>

namespace libcage {
        void
        timer_callback(int fd, short event, void *arg)
        {
                timer::callback &func = *(timer::callback*)arg;
                timer *t = func.get_timer();

                t->m_events.erase(&func);

                func();
        }

        void timer_callback_ev(EV_P_ ev_timer *w, int revents) 
        {
                timer_callback(0, revents, w->data);
        }

        timer::~timer()
        {
                boost::unordered_map<callback*,
                        boost::shared_ptr<ev_timer> >::iterator it;

                for (it = m_events.begin(); it != m_events.end(); ++it) {
                        ev_timer_stop(EV_DEFAULT_UC_ it->second.get());
                }
        }

        void
        timer::set_timer(callback *func, timeval *t)
        {
                typedef boost::shared_ptr<ev_timer> ev_ptr;
                ev_ptr ev = ev_ptr(new ev_timer);
                ev_tstamp repeat = (double) t->tv_sec;

                // delete old event
                unset_timer(func);

                func->m_timer = this;

                // add new event
                m_events[func] = ev;
                ev->data = func;
                ev_timer_init(ev.get(), timer_callback_ev, 0., repeat);
        }

        void
        timer::unset_timer(callback *func)
        {
                if (m_events.find(func) != m_events.end()) {
                        ev_timer_stop(EV_DEFAULT_UC_ m_events[func].get());
                        m_events.erase(func);
                }
        }

#ifdef DEBUG
        class timer_func : public timer::callback {
        public:
                virtual void operator() () {
                        printf("test timer: ok\n");
                }
        };

        void
        timer::test_timer()
        {
                timeval tval1;

                tval1.tv_sec  = 2;
                tval1.tv_usec = 0;

                timer      *t;
                timer_func *func;

                t    = new timer();
                func = new timer_func();

                t->set_timer(func, &tval1);
        }
#endif // DEBUG
}
