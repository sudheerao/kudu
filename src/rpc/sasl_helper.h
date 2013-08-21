// Copyright (c) 2013, Cloudera, inc.
// All rights reserved.

#ifndef KUDU_RPC_SASL_HELPER_H
#define KUDU_RPC_SASL_HELPER_H

#include <set>
#include <string>

#include <sasl/sasl.h>

#include "gutil/gscoped_ptr.h"
#include "gutil/macros.h"
#include "util/net/socket.h"

namespace google {
namespace protobuf {
class MessageLite;
} // namespace
} // namespace

namespace kudu {

class Sockaddr;
class Status;

namespace rpc {

using std::set;
using std::string;

class SaslMessagePB;

// Helper class which contains functionality that is common to SaslClient & SaslServer.
// Most of these methods are convenience methods for interacting with the libsasl2 library.
class SaslHelper {
 public:
  enum PeerType {
    CLIENT,
    SERVER
  };

  explicit SaslHelper(PeerType peer_type);
  ~SaslHelper();

  // Specify IP:port of local side of connection.
  void set_local_addr(const Sockaddr& addr);
  const char* local_addr_string() const;

  // Specify IP:port of remote side of connection.
  void set_remote_addr(const Sockaddr& addr);
  const char* remote_addr_string() const;

  // Specify the fully-qualified domain name of the remote server.
  void set_server_fqdn(const string& domain_name);
  const char* server_fqdn() const;

  // Globally-registered available SASL plugins.
  const set<string>& GlobalMechs() const;

  // Helper functions for managing the list of active SASL mechanisms.
  void AddToLocalMechList(const string& mech);
  const set<string>& LocalMechs() const;

  // Returns space-delimited local mechanism list string suitable for passing
  // to libsasl2, such as via "mech_list" callbacks.
  // The returned pointer is valid only until the next call to LocalMechListString().
  const char* LocalMechListString() const;

  // Implements the client_mech_list / mech_list callbacks.
  int GetOptionCb(const char* plugin_name, const char* option, const char** result, unsigned* len);

  // Enable the ANONYMOUS SASL mechanism.
  Status EnableAnonymous();

  // Check for the ANONYMOUS SASL mechanism.
  bool IsAnonymousEnabled() const;

  // Enable the PLAIN SASL mechanism.
  Status EnablePlain();

  // Check for the PLAIN SASL mechanism.
  bool IsPlainEnabled() const;

  // Sanity check that the call ID is the SASL call ID.
  // Logs DFATAL if call_id does not match.
  Status SanityCheckSaslCallId(int32_t call_id) const;

  // Parse msg from the given Slice.
  Status ParseSaslMessage(const Slice& param_buf, SaslMessagePB* msg);

  // Encode and send a message over a socket.
  Status SendSaslMessage(Socket* sock, const google::protobuf::MessageLite& header,
      const google::protobuf::MessageLite& msg);

  // Receive a full message frame from the server.
  // recv_buf: buffer to use for reading the data from the socket.
  // header: Response header protobuf.
  // param_buf: Slice into recv_buf containing unparsed RPC param protobuf data.
  Status ReceiveFramedMessage(Socket* sock, faststring* recv_buf,
      google::protobuf::MessageLite* header, Slice* param_buf);

 private:
  string local_addr_;
  string remote_addr_;
  string server_fqdn_;

  // Authentication types and data.
  const PeerType peer_type_;
  bool conn_header_exchanged_;
  string tag_;
  mutable gscoped_ptr< set<string> > global_mechs_;  // Cache of global mechanisms.
  set<string> mechs_;    // Active mechanisms.
  mutable string mech_list_;  // Mechanism list string returned by callbacks.

  bool anonymous_enabled_;
  bool plain_enabled_;

  DISALLOW_COPY_AND_ASSIGN(SaslHelper);
};

} // namespace rpc
} // namespace kudu

#endif  // KUDU_RPC_SASL_HELPER_H
