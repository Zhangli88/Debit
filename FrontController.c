#import "FrontController.h"
#import "RequestPacket.h"

#define self FrontController

rsdef(self, New, Logger *logger) {
	return (self) {
		.route    = NULL,
		.resource = NULL,
		.instance = Generic_Null(),
		.logger   = logger
	};
}

static def(void, DestroyResource) {
	if (this->resource->destroy != NULL) {
		this->resource->destroy(this->instance);
	}

	fwd(i, ResourceInterface_MaxMembers) {
		ResourceMember *member = &this->resource->members[i];

		if (member->name.buf == NULL) {
			break;
		}

		if (member->array) {
			StringArray **s = Generic_GetObject(this->instance) + member->offset;
			StringArray_Destroy(*s);
			StringArray_Free(*s);
		} else {
			String *s = Generic_GetObject(this->instance) + member->offset;
			String_Destroy(s);
		}
	}

	if (!Generic_IsNull(this->instance)) {
		Generic_Free(this->instance);
	}
}

def(void, Destroy) {
	if (this->resource != NULL) {
		call(DestroyResource);
	}
}

def(bool, HasResource) {
	return this->resource != NULL;
}

static def(ResourceMember *, ResolveMember, RdString name) {
	fwd(i, ResourceInterface_MaxMembers) {
		ResourceMember *member = &this->resource->members[i];

		if (member->name.buf == NULL) {
			break;
		}

		if (String_Equals(member->name, name)) {
			return member;
		}
	}

	return NULL;
}

def(String *, GetMemberAddr, RdString name) {
	if (call(HasResource)) {
		ResourceMember *member = call(ResolveMember, name);

		if (member != NULL) {
			if (!member->array) {
				return Generic_GetObject(this->instance) + member->offset;
			} else {
				StringArray **ptr = Generic_GetObject(this->instance) + member->offset;
				StringArray_Push(ptr, String_New(0));
				return &(*ptr)->buf[(*ptr)->len - 1];
			}
		}
	}

	return NULL;
}

def(bool, Store, RdString name, RdString value) {
	String *s = call(GetMemberAddr, name);

	if (s != NULL) {
		String_Copy(s, value);
		return true;
	}

	return false;
}

def(void, StoreEx, RdString name, RdString value) {
	String *s = call(GetMemberAddr, name);

	if (s != NULL) {
		String_Copy(s, value);
	}
}

def(void, CreateResource, ResourceRoute *route, ResourceInterface *resource) {
	/* This function should only be called once in order not to leak memory. */
	assert(this->route    == NULL);
	assert(this->resource == NULL);
	assert(Generic_IsNull(this->instance));

	this->route    = route;
	this->resource = resource;
	this->instance = Generic_New(this->resource->size);

	fwd(i, ResourceInterface_MaxMembers) {
		ResourceMember *member = &this->resource->members[i];

		if (member->name.buf == NULL) {
			break;
		}

		if (member->array) {
			StringArray **ptr = Generic_GetObject(this->instance) + member->offset;
			*ptr = StringArray_New(16);
		} else {
			String *ptr = Generic_GetObject(this->instance) + member->offset;
			*ptr = String_New(0);
		}
	}

	if (this->resource->init != NULL) {
		this->resource->init(this->instance);
	}
}

def(void, Dispatch, Session *sess, Request *request, Response *response) {
	assert(sess     != NULL);
	assert(request  != NULL);
	assert(response != NULL);

	if (!call(HasResource)) {
		Response_SetStatus(response, HTTP_Status_ClientError_NotFound);

		Response_SetBufferBody(response,
			String_ToCarrier($$("Sorry, no matching route found")));

		Response_Flush(response);

		return;
	}

	assert(this->route != NULL);
	assert(!Generic_IsNull(this->instance));

	if (this->route->role == Role_Unspecified) {
		if (this->resource->role == Role_Unspecified) {
			/* Default role. Page is accessible for anyone. */
			this->route->role = Role_Guest;
		} else {
			/* Inherit role. */
			this->route->role = this->resource->role;
		}
	}

	if (this->route->role == Role_User /* && !Session_IsUser(sess) */) {
		/* Authorization required. */
		Logger_Debug(this->logger, $("Authorization required"));
	} else {
		/* Rule doesn't require user role or client is already
		 * authorized.
		 */

		if (this->route->setUp != NULL) {
			this->route->setUp(this->instance, sess, request, response);
		}

		#undef action

		try {
			this->route->action(this->instance, sess, request, response);
		} catchAny {
			String fmt = Exception_Format(e);
			Logger_Debug(this->logger, fmt.rd);

			Response_SetStatus(response, HTTP_Status_ServerError_Internal);
			Response_SetBufferBody(response, String_ToCarrier(fmt));
			Response_Flush(response);

#if Exception_SaveTrace
			Backtrace_PrintTrace(
				Exception_GetTraceBuffer(),
				Exception_GetTraceLength());
#endif
		} finally {

		} tryEnd;
	}
}

def(void, PostDispatch, Session *sess, Request *request, Response *response) {
	assert(sess     != NULL);
	assert(request  != NULL);
	assert(response != NULL);

	if (call(HasResource)) {
		assert(this->route != NULL);
		assert(!Generic_IsNull(this->instance));

		if (this->route->tearDown != NULL) {
			this->route->tearDown(this->instance, sess, request, response);
		}
	}
}

def(void, Error, HTTP_Status status, RdString msg, Response *response) {
	Logger_Error(this->logger, $("Client error: %"), msg);

	HTTP_Status_Item st = HTTP_Status_GetItem(status);

	String strCode = Integer_ToString(st.code);

	Response_SetStatus    (response, status);
	Response_SetBufferBody(response, String_ToCarrier(String_Format(
		$(
			"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" "
								"\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
			"<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">"
					"<head>"
							"<title>% - %</title>"
					"</head>"
					"<body>"
							"<h1>% - %</h1>"
							"<h2>%</h2>"
					"</body>"
			"</html>"),

		strCode.rd, st.msg,
		strCode.rd, st.msg,
		msg)));

	String_Destroy(&strCode);

	Response_Flush(response);
}
