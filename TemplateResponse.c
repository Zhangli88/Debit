#import "TemplateResponse.h"

void TemplateResponse(Response *resp, Template tpl) {
	String out = String_New(0);

	callback(tpl, &out);

	Response_SetBufferBody (resp, String_ToCarrier(out));
	Response_SetContentType(resp, String_ToCarrier($$("text/html; charset=utf-8")));

	Response_Flush(resp);
}
