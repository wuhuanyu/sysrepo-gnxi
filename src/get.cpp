/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <grpc/grpc.h>

#include "server.h"

using namespace std;
using google::protobuf::RepeatedPtrField;
using sysrepo::sysrepo_exception;

/*
 * Build Get Notifications
 * Contrary to Subscribe Notifications, A new notification message must be
 * created for every path of the GetRequest.
 * There can still be multiple paths in GetResponse if requested path
 * is a directory path.
 *
 * IMPORTANT : we have choosen to have a stateless implementation of
 * gNMI so deleted path in Notification message will always be empty.
 */
Status
GNMIServer::BuildGetNotification(Notification *notification, const Path *prefix,
                                 Path &path, gnmi::Encoding encoding)
{
  /* Data elements that have changed values */
  RepeatedPtrField<Update>* updateList = notification->mutable_update();
  Update *update;
  TypedValue *gnmival;
  string fullpath = "";

  /* Get time since epoch in milliseconds */
  notification->set_timestamp(get_time_nanosec());

  /* Put Request prefix as Response prefix */
  if (prefix != nullptr) {
    string str = gnmi_to_xpath(*prefix);
    cerr << "DEBUG: prefix is" << str << endl;
    notification->mutable_prefix()->CopyFrom(*prefix);
    fullpath += str;
  }

  fullpath += gnmi_to_xpath(path);
  cout << "DEBUG: GetRequest Path " << fullpath << endl;

  /* Create Update message */
  update = updateList->Add();
  update->mutable_path()->CopyFrom(path);
  gnmival = update->mutable_val();

  /* Refresh configuration data from current session */
  sr_sess->refresh();

  /* Create appropriate TypedValue message based on encoding */
  if (encoding == JSON) {
    gnmival->mutable_json_ietf_val(); //TODO return a string*
    /* Get sysrepo subtree data corresponding to XPATH */
    try {
      string tmp = encodef->getEncoding(EncodeFactory::Encoding::JSON)->read(fullpath);
      cout << tmp << endl;
    } catch (invalid_argument &exc) {
      return Status(StatusCode::NOT_FOUND, exc.what());
    } catch (sysrepo_exception &exc) {
      cerr << "ERROR: Fail getting items from sysrepo "
           << "l." << __LINE__ << " " << exc.what()
           << endl;
      return Status(StatusCode::INVALID_ARGUMENT, exc.what());
    }
  } else {
    return Status(StatusCode::UNIMPLEMENTED, Encoding_Name(encoding));
  }


  /* TODO Check DATA TYPE in {ALL,CONFIG,STATE,OPERATIONAL}
   * This is interesting for NMDA architecture
   * req->type() : GetRequest_DataType_ALL,CONFIG,STATE,OPERATIONAL
   */

  cout << "DEBUG: End of Notification" << endl;

  return Status::OK;
}

/* Verify request fields are correct */
static inline Status verifyGetRequest(const GetRequest *request)
{
  if (request->encoding() != JSON) {
    cerr << "WARN: Unsupported Encoding" << endl;
    return Status(StatusCode::UNIMPLEMENTED, Encoding_Name(request->encoding()));
  }

  if (!GetRequest_DataType_IsValid(request->type())) {
    cerr << "WARN: invalid Data Type in Get Request" << endl;
    return Status(StatusCode::UNIMPLEMENTED,
                  GetRequest_DataType_Name(request->type()));
  }

  if (request->use_models_size() > 0) {
    cerr << "WARN: Use models feature unsupported, ALL are used" << endl;
    return Status(StatusCode::UNIMPLEMENTED,
                  grpc::string("use_model feature unsupported"));
  }

  if (request->extension_size() > 0) {
    cerr << "WARN: extension unsupported" << endl;
    return Status(StatusCode::UNIMPLEMENTED,
                  grpc::string("extension feature unsupported"));
  }

  return Status::OK;
}

/* Implement gNMI Get RPC */
Status GNMIServer::Get(ServerContext *context, const GetRequest* req,
                        GetResponse* response)
{
  UNUSED(context);
  RepeatedPtrField<Notification> *notificationList;
  Notification *notification;
  Status status;

  status = verifyGetRequest(req);
  if (!status.ok())
    return status;

  cout << "DEBUG: GetRequest DataType " << GetRequest::DataType_Name(req->type())
      << endl;

  cout << "DEBUG: GetRequest Encoding " << Encoding_Name(req->encoding())
       << endl;

  /* Run through all paths */
  notificationList = response->mutable_notification();
  for (auto path : req->path()) {
    notification = notificationList->Add();

    if (req->has_prefix())
      status = BuildGetNotification(notification, &req->prefix(), path, req->encoding());
    else
      status = BuildGetNotification(notification, nullptr, path, req->encoding());

    if (!status.ok()) {
      cerr << "ERROR:"
           << " Errcode=" << status.error_code()
           << " Errmessage=" << status.error_message()
           << endl;
      return status;
    }
  }

  return Status::OK;
}
