#import "BufferResponse.h"

void BufferResponse(ResponseInstance resp, String buf) {
	Response_SetBufferBody(resp, buf);
	Response_SetContentType(resp, $("text/html; charset=utf-8"));
}
