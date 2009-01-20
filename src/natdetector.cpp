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

#include "natdetector.hpp"
#include "cagetypes.hpp"

namespace libcage {
        const time_t    natdetector::echo_timeout = 3;

        void
        natdetector::timer_echo_wait1::operator() ()
        {
                m_nat->m_state = undefined;

                // delete this
                // do not reference menber variables after this code
                m_nat->m_timers.erase(m_nonce);
        }

        void
        natdetector::timer_echo_wait2::operator() ()
        {
                m_nat->m_state = nat;
                m_nat->m_reply.erase(m_nonce);

                // delete this
                // do not reference menber variables after this code
                m_nat->m_timers.erase(m_nonce);
        }

        void
        natdetector::udp_receiver::operator() (udphandler &udp, void *buf,
                                               int len, sockaddr *from,
                                               int fromlen, bool is_timeout)
        {
                if (is_timeout) {
                        m_nat->m_state = undefined;
                        m_udp.close();

                        // delete this
                        // do not reference menber variables after this code
                        m_nat->m_udps.erase(m_nonce);
                } else {
                        if (len == (int)sizeof(msg_nat_echo_redirect_reply)) {
                                msg_hdr *hdr = (msg_hdr*)buf;

                                if (hdr->type ==
                                    type_nat_echo_redirect_reply) {
#ifdef DEBUG_NAT
                                        printf("recv nat_echo_redirect_reply\n");
#endif // DEBUG_NAT
                                        m_nat->recv_echo_redirect_reply(buf);
                                }
                        } else {
                                m_nat->m_state = undefined;
                        }

                        m_udp.close();

                        // delete this
                        // do not reference menber variables after this code
                        m_nat->m_udps.erase(m_nonce);
                }
        }

        natdetector::natdetector(udphandler &udp, timer &t,
                                 const uint160_t &id) :
                m_state(undefined), m_timer(t), m_udp(udp), m_id(id),
                m_global_port(0)
        {

        }

        natdetector::~natdetector()
        {
        
        }

        void
        natdetector::detect_nat(std::string host, int port)
        {
                if (m_state != undefined) {
                        return;
                }

                // send echo
                msg_nat_echo echo;

                memset(&echo, 0, sizeof(echo));

                echo.hdr.magic = htons(MAGIC_NUMBER);
                echo.hdr.ver   = CAGE_VERSION;
                echo.hdr.type  = type_nat_echo;
                m_id.to_binary(echo.hdr.src, sizeof(echo.hdr.src));

                uint32_t nonce;
                for (;;) {
                        nonce = (uint32_t)mrand48();
                        if (m_timers.find(nonce) == m_timers.end())
                                break;
                }

                echo.nonce = htonl(nonce);


                // set timer
                timeval tval;

                tval.tv_sec  = echo_timeout;
                tval.tv_usec = 0;

                callback_ptr func(new timer_echo_wait1);
                timer_echo_wait1 *p = (timer_echo_wait1*)func.get();

                p->m_nonce = nonce;
                p->m_nat   = this;

                m_timer.set_timer(func.get(), &tval);


                // send
                m_udp.sendto(&echo, sizeof(echo), host, port);


                m_timers[nonce] = func;
                m_state = echo_wait1;
        }

        void
        natdetector::recv_echo(void *msg, sockaddr *from, int fromlen)
        {
                msg_nat_echo *echo = (msg_nat_echo*)msg;
                msg_nat_echo_reply reply;

                memset(&reply, 0, sizeof(reply));

                reply.hdr.magic = htons(MAGIC_NUMBER);
                reply.hdr.ver   = CAGE_VERSION;
                reply.hdr.type  = type_nat_echo_reply;

                m_id.to_binary(reply.hdr.src, sizeof(reply.hdr.src));
                memcpy(reply.hdr.dst, echo->hdr.src, sizeof(reply.hdr.dst));

                reply.nonce  = echo->nonce;

                if (from->sa_family == PF_INET) {
                        sockaddr_in *saddr = (sockaddr_in*)from;

                        reply.domain = htons(domain_inet);
                        reply.port   = saddr->sin_port;
                        memcpy(reply.addr, &saddr->sin_addr,
                               sizeof(saddr->sin_addr));
                } else if (from->sa_family == PF_INET6) {
                        sockaddr_in6 *saddr = (sockaddr_in6*)from;

                        reply.domain = htons(domain_inet6);
                        reply.port   = saddr->sin6_port;
                        memcpy(reply.addr, &saddr->sin6_addr,
                               sizeof(saddr->sin6_addr));
                }

                m_udp.sendto(&reply, sizeof(reply), from, fromlen);
        }

        void
        natdetector::recv_echo_reply_wait1(void *msg, sockaddr *from,
                                           int fromlen)
        {
                msg_nat_echo_reply *reply = (msg_nat_echo_reply*)msg;
                int                 nonce1 = ntohl(reply->nonce);

                if (m_timers.find(nonce1) == m_timers.end())
                        return;

                m_timer.unset_timer(m_timers[nonce1].get());
                m_timers.erase(nonce1);

                // open another UDP socket
                timeval       t;
                udp_ptr       udp(new udp_receiver);
                udp_receiver *p_udp = (udp_receiver*)udp.get();
                uint32_t      nonce2;

                for (;;) {
                        nonce2 = (uint32_t)mrand48();
                        if (m_udps.find(nonce2) == m_udps.end())
                                break;
                }
                
                t.tv_sec  = echo_timeout;
                t.tv_usec = 0;

                p_udp->m_udp.open(from->sa_family, 0);
                p_udp->m_udp.set_callback(udp.get(), &t);

                p_udp->m_nonce = nonce2;
                p_udp->m_nat   = this;

                m_udps[nonce2] = udp;


                // send echo_redirect
                msg_nat_echo_redirect redirect;

                memset(&redirect, 0, sizeof(redirect));
                
                redirect.hdr.magic = htons(MAGIC_NUMBER);
                redirect.hdr.ver   = CAGE_VERSION;
                redirect.hdr.type  = type_nat_echo_redirect;

                m_id.to_binary(redirect.hdr.src, sizeof(redirect.hdr.src));
                memcpy(redirect.hdr.dst, reply->hdr.src,
                       sizeof(redirect.hdr.dst));

                redirect.nonce = htonl(nonce2);
                redirect.port  = p_udp->m_udp.get_port();

                m_udp.sendto(&redirect, sizeof(redirect), from, fromlen);


                m_state = echo_redirect_wait;
        }

        void
        natdetector::recv_echo_reply_wait2(void *msg)
        {
                msg_nat_echo_reply *reply = (msg_nat_echo_reply*)msg;

                uint32_t nonce = ntohl(reply->nonce);
                if (m_timers.find(nonce) == m_timers.end())
                        return;

                if (m_reply[nonce] == 0) {
                        m_reply[nonce] = reply->port;
                } else if (m_reply[nonce] == reply->port) {
                        m_state = cone_nat;

                        m_timer.unset_timer(m_timers[nonce].get());

                        m_reply.erase(nonce);
                        m_timers.erase(nonce);

#ifdef DEBUG_NAT
                        printf("cone nat\n");
#endif // DEBUG_NAT
                } else {
                        m_state = symmetric_nat;

                        m_timer.unset_timer(m_timers[nonce].get());

                        m_reply.erase(nonce);
                        m_timers.erase(nonce);
                }
        }

        void
        natdetector::recv_echo_reply(void *msg, sockaddr *from, int fromlen)
        {
                if (m_state == echo_wait1)
                        recv_echo_reply_wait1(msg, from, fromlen);

                if (m_state == echo_wait2)
                        recv_echo_reply_wait2(msg);
        }

        void
        natdetector::recv_echo_redirect(void *msg, sockaddr *from, int fromlen)
        {
                msg_nat_echo_redirect      *redirect;
                msg_nat_echo_redirect_reply reply;

                memset(&reply, 0, sizeof(reply));

                redirect = (msg_nat_echo_redirect*)msg;

                reply.hdr.magic = htons(MAGIC_NUMBER);
                reply.hdr.ver   = CAGE_VERSION;
                reply.hdr.type  = type_nat_echo_redirect_reply;

                m_id.to_binary(reply.hdr.src, sizeof(reply.hdr.src));
                memcpy(reply.hdr.dst, redirect->hdr.src, sizeof(reply.hdr.dst));

                reply.nonce = redirect->nonce;

                if (from->sa_family == PF_INET) {
                        sockaddr_in *in = (sockaddr_in*)from;

                        reply.port   = in->sin_port;
                        reply.domain = domain_inet;
                        memcpy(reply.addr, &in->sin_addr, sizeof(in->sin_addr));
                        
                        in->sin_port = redirect->port;
                } else if (from->sa_family == PF_INET6) {
                        sockaddr_in6 *in6 = (sockaddr_in6*)from;

                        reply.port     = in6->sin6_port;
                        reply.domain = domain_inet6;
                        memcpy(reply.addr, &in6->sin6_addr,
                               sizeof(in6->sin6_addr));

                        in6->sin6_port = redirect->port;
                }

                m_udp.sendto(&reply, sizeof(reply), from, fromlen);
        }

        void
        natdetector::recv_echo_redirect_reply(void *msg)
        {
                if (m_state != echo_redirect_wait) {
                        m_state = undefined;
                        return;
                }

                msg_nat_echo_redirect_reply *reply;
                uint32_t                     nonce;

                reply = (msg_nat_echo_redirect_reply*)msg;
                nonce = ntohl(reply->nonce);

                if (m_udps.find(nonce) == m_udps.end()) {
                        m_state = undefined;
                        return;
                }

                m_global_port = reply->port;
                memcpy(m_global_addr, reply->addr, sizeof(m_global_addr));

                m_state = nat;
        }

        void
        natdetector::detect_nat_type(std::string host1, int port1,
                                     std::string host2, int port2)
        {
                if (m_state != nat)
                        return;

                // send echo
                msg_nat_echo echo;

                memset(&echo, 0, sizeof(echo));

                echo.hdr.magic = htons(MAGIC_NUMBER);
                echo.hdr.ver   = CAGE_VERSION;
                echo.hdr.type  = type_nat_echo;
                m_id.to_binary(echo.hdr.src, sizeof(echo.hdr.src));

                uint32_t nonce;
                for (;;) {
                        nonce = (uint32_t)mrand48();
                        if (m_timers.find(nonce) == m_timers.end())
                                break;
                }

                echo.nonce = htonl(nonce);


                // set timer
                timeval tval;

                tval.tv_sec  = echo_timeout;
                tval.tv_usec = 0;

                callback_ptr func(new timer_echo_wait2);
                timer_echo_wait2 *p = (timer_echo_wait2*)func.get();

                p->m_nonce = nonce;
                p->m_nat   = this;

                m_timer.set_timer(func.get(), &tval);


                // send
                m_udp.sendto(&echo, sizeof(echo), host1, port1);
                m_udp.sendto(&echo, sizeof(echo), host2, port2);


                m_timers[nonce] = func;
                m_reply[nonce]  = 0;
                m_state         = echo_wait2;
        }
}
