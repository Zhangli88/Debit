#import "BufferResponse.h"

overload void BufferResponse(ResponseInstance resp, String buf) {
	Response_SetBufferBody(resp, String_ToCarrier(buf));
	Response_SetContentType(resp, String_ToCarrier($("text/html; charset=utf-8")));
}

overload void BufferResponse(ResponseInstance resp, ProtString buf) {
	Response_SetBufferBody(resp, String_ToCarrier(buf));
	Response_SetContentType(resp, String_ToCarrier($("text/html; charset=utf-8")));
}
