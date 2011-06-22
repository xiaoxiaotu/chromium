// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_PLUGINS_PPAPI_PPB_TRANSPORT_IMPL_H_
#define WEBKIT_PLUGINS_PPAPI_PPB_TRANSPORT_IMPL_H_

#include <list>
#include <string>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "net/base/completion_callback.h"
#include "ppapi/thunk/ppb_transport_api.h"
#include "webkit/glue/p2p_transport.h"
#include "webkit/plugins/ppapi/callbacks.h"
#include "webkit/plugins/ppapi/resource.h"

namespace webkit {
namespace ppapi {

class PPB_Transport_Impl : public Resource,
                           public ::ppapi::thunk::PPB_Transport_API,
                           public webkit_glue::P2PTransport::EventHandler {
 public:
  virtual ~PPB_Transport_Impl();

  static PP_Resource Create(PluginInstance* instance,
                            const char* name,
                            const char* proto);

  // ResourceObjectBase override.
  virtual ::ppapi::thunk::PPB_Transport_API* AsPPB_Transport_API() OVERRIDE;

  // PPB_Transport_API implementation.
  virtual PP_Bool IsWritable() OVERRIDE;
  virtual int32_t Connect(PP_CompletionCallback callback) OVERRIDE;
  virtual int32_t GetNextAddress(PP_Var* address,
                                 PP_CompletionCallback callback) OVERRIDE;
  virtual int32_t ReceiveRemoteAddress(PP_Var address) OVERRIDE;
  virtual int32_t Recv(void* data, uint32_t len,
                       PP_CompletionCallback callback) OVERRIDE;
  virtual int32_t Send(const void* data, uint32_t len,
                       PP_CompletionCallback callback) OVERRIDE;
  virtual int32_t Close() OVERRIDE;

  // webkit_glue::P2PTransport::EventHandler implementation.
  virtual void OnCandidateReady(const std::string& address) OVERRIDE;
  virtual void OnStateChange(webkit_glue::P2PTransport::State state) OVERRIDE;
  virtual void OnError(int error) OVERRIDE;

 private:
  explicit PPB_Transport_Impl(PluginInstance* instance);

  bool Init(const char* name, const char* proto);

  void OnRead(int result);
  void OnWritten(int result);

  std::string name_;
  bool use_tcp_;
  bool started_;
  scoped_ptr<webkit_glue::P2PTransport> p2p_transport_;
  bool writable_;
  std::list<std::string> local_candidates_;

  scoped_refptr<TrackedCompletionCallback> connect_callback_;
  scoped_refptr<TrackedCompletionCallback> next_address_callback_;

  scoped_refptr<TrackedCompletionCallback> recv_callback_;
  scoped_refptr<TrackedCompletionCallback> send_callback_;

  net::CompletionCallbackImpl<PPB_Transport_Impl> channel_write_callback_;
  net::CompletionCallbackImpl<PPB_Transport_Impl> channel_read_callback_;

  DISALLOW_COPY_AND_ASSIGN(PPB_Transport_Impl);
};

}  // namespace ppapi
}  // namespace webkit

#endif  // WEBKIT_PLUGINS_PPAPI_PPB_TRANSPORT_IMPL_H_
