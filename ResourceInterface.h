#import <HTTP.h>
#import <String.h>
#import <HTTP/Method.h>

#import "Session.h"
#import "Response.h"

#ifndef ResourceInterface_MaxMembers
#define ResourceInterface_MaxMembers 30
#endif

#ifndef ResourceInterface_MaxRoutes
#define ResourceInterface_MaxRoutes  15
#endif

record(Request) {
	struct {
		String referer;
		String sessionId;
		HTTP_Method method;
		Date_RFC822 lastModified;
	} priv;
};

static alwaysInline RdString Request_GetReferer(Request req) {
	return req.priv.referer.rd;
}

static alwaysInline RdString Request_GetSessionId(Request req) {
	return req.priv.sessionId.rd;
}

static alwaysInline HTTP_Method Request_GetMethod(Request req) {
	return req.priv.method;
}

static alwaysInline Date_RFC822 Request_GetLastModified(Request req) {
	return req.priv.lastModified;
}

typedef void (ResourceInit)   (GenericInstance);
typedef void (ResourceDestroy)(GenericInstance);
typedef void (ResourceAction) (GenericInstance $this,
	Session  *      $sess,
	Request         $req,
	Response *      $resp);

#define action(name)            \
	static def(void, name,      \
		__unused Session *sess, \
		__unused Request req,   \
		__unused Response *resp)

#define dispatch(name) \
	call(name, sess, req, resp)

#define Action(name) \
	(void *) ref(name)

#define RouterConstructor                               \
	Constructor {                                       \
		Router *router = Router_GetInstance();          \
		Router_AddResource(router, &ref(ResourceImpl)); \
	}

record(ResourceMember) {
	RdString name;
	size_t offset;
	bool array;
};

#define Member(name) \
	offsetof(self, name)

set(Role) {
	/* This is the default value when the `role' field is not set in
	 * the resource structure. The page can be accessed by anyone.
	 */

	Role_Unspecified,

	/* The user is unauthorized. */
	Role_Guest,

	/* The user is logged in. */
	Role_User
};

record(ResourceRoute) {
	RdString path;
	ResourceAction *action;

	/* If unspecified, the resource's role is inherited. */
	Role role;

	ResourceAction *setUp;
	ResourceAction *tearDown;
};

Interface(Resource) {
	Role role;

	size_t   size;
	RdString name;

	ResourceInit    *init;
	ResourceDestroy *destroy;

	ResourceRoute  routes  [ResourceInterface_MaxRoutes ];
	ResourceMember members [ResourceInterface_MaxMembers];
};
