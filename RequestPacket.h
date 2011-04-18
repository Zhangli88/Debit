#import "Request.h"
#import "Response.h"
#import "SessionManager.h"
#import "FrontController.h"
#import "DoublyLinkedList.h"

#define self RequestPacket

set(ref(State)) {
	ref(State_Processing),
	ref(State_Done)
};

struct ResponseSender;

/* Aggregates the request and its corresponding response. */
class {
	DoublyLinkedList_DeclareRef(self);

	Logger  *logger;
	Session *sess;

	ref(State) state;

	Request  request;
	Response response;

	FrontController controller;

	struct ResponseSender *sender;
};

DoublyLinkedList_DeclareList(RequestPacket, RequestPackets);

def(void, Init, struct ResponseSender *sender, Logger *logger);
def(void, Destroy);
def(void, SetVersion, HTTP_Version method);
def(void, SetMethod, HTTP_Method method);
def(void, SetCookie, RdString name, RdString value);
def(void, SetHeader, RdString name, RdString value);
def(void, Dispatch, bool persistent);
def(void, Flush);
def(bool, IsReady);

#undef self
