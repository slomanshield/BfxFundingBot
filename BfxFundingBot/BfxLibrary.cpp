#include "BfxLibrary.h"

BfxLibrary::BfxLibrary()
{
	apiKey = "";
	secretKey = "";
	responseData = "";
	

	webSocket.setUrl(WebSocketURL);
	webSocket.setOnMessageCallback(recieveCallBack);
	webSocket.setHeartBeatPeriod(5);
	curl = NULL;
}

BfxLibrary::~BfxLibrary()
{
	if(webSocket.getReadyState() == WebSocket_ReadyState_Open)
		webSocket.stop();
	if (curl != NULL)
		curl_easy_cleanup(curl);
	return;
}

bool BfxLibrary::dataReceived = false;
std::string BfxLibrary::webSocketRecievedData = "";

void BfxLibrary::recieveCallBack(ix::WebSocketMessageType messageType,
	const std::string& str,
	size_t wireSize,
	const WebSocketErrorInfo& error,
	const WebSocketOpenInfo& headers,
	const WebSocketCloseInfo& closeInfo)
{
	if (messageType == ix::WebSocket_MessageType_Message)
	{
		if (str.find("\"auth\"") != -1)//we have auth data
		{
			BfxLibrary::webSocketRecievedData = str.c_str();
			BfxLibrary::dataReceived = true;
		}
		else if (str.find("\"fon-req\"") != -1)//we have request data from funding
		{
			BfxLibrary::webSocketRecievedData = str.c_str();
			BfxLibrary::dataReceived = true;
		}
		else if (str.find("\"foc\"") != -1)//we have request data from cancel (could be a successful cancel)
		{
			BfxLibrary::webSocketRecievedData = str.c_str();
			BfxLibrary::dataReceived = true;
		}
	}
}

int BfxLibrary::WaitForData(double timeoutMilliseconds)
{
	HighResolutionTimePoint startTime = std::chrono::high_resolution_clock::now();
	HighResolutionTimePoint currTime = startTime;
	int timeout = SUCCESS;
	DoubleMili diff;

	while (BfxLibrary::dataReceived == false)
	{
		if (timeoutMilliseconds > 0)//if 0 or less wait indefinetly
		{
			diff = std::chrono::duration_cast<std::chrono::milliseconds>(currTime - startTime);
			if (diff.count() > timeoutMilliseconds)
			{
				timeout = WebSocketGetDataTimeout;
				break;
			}
				
		}
		usleep(10000);//sleep for 10 miliseconds
		currTime = std::chrono::high_resolution_clock::now();
	}
	return timeout;
}

void BfxLibrary::InitKeys(char* apiKey, char* secretKey)
{
	this->apiKey = apiKey;
	this->secretKey = secretKey;
}

int BfxLibrary::ConnectWebSocket()
{
	int cc = SUCCESS;
	ix::WebSocketInitResult initResult;
	initResult = webSocket.connect(5000);

	if (initResult.success == false)
	{
		LOG_ERROR(initResult.errorStr);
		cc = WebSocketConnectFailure;
	}
	else
	{
		webSocket.start();
	}

	return cc;
}

int BfxLibrary::SendAuthWebSocket()
{
	int cc = SUCCESS;
 
	if (webSocket.getReadyState() == WebSocket_ReadyState_Open)
	{
		std::string authInput = CreateWebSocketAuthPayload();
		Document doc;

		cc = SendRecieveWebSocketData(&authInput, &responseData);

		if (cc == SUCCESS)
		{
			cc = ParseResponseData(&doc);
		}

		if (cc == SUCCESS)
		{
			if (doc.HasMember("status") && doc["status"].IsString())
			{
				authStatus = doc["status"].GetString();
				if (authStatus.compare("OK") != SUCCESS)
				{
					LOG_ERROR(string_format(format_string_auth, authStatus));
					cc = ERROR_AUTH;
				}
			}
		}


	}
	else
		cc = WebSocketNotConnected;
	return cc;
}

int BfxLibrary::DisconnetWebSocket()
{
	webSocket.stop();
	return SUCCESS;
}

int BfxLibrary::SendRecieveWebSocketData(std::string* in, std::string* out)
{
	int cc = SUCCESS;
	
	if (webSocket.getReadyState() == WebSocket_ReadyState_Open)
	{
		BfxLibrary::dataReceived = false;
		webSocket.send(in->c_str());

		cc = WaitForData(5000);
		if (cc == SUCCESS)
		{
			*out = BfxLibrary::webSocketRecievedData;
		}
		else
		{
			LOG_ERROR_CODE(cc);
		}
	}
	else
		cc = WebSocketNotConnected;

	return cc;
}

void BfxLibrary::InitCurl()
{
	
	if (curl == NULL)
		curl = curl_easy_init();
	else
		curl_easy_reset(curl);


	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
	}
}

std::string BfxLibrary::GetBase64(std::string input)
{
	std::string output;
	BIO *bio, *b64;
	BUF_MEM *bufferPtr;

	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new(BIO_s_mem());
	bio = BIO_push(b64, bio);

	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Ignore newlines - write everything in one line
	BIO_write(bio, input.c_str(), input.length());
	BIO_flush(bio);
	BIO_get_mem_ptr(bio, &bufferPtr);
	
	output.append((*bufferPtr).data, (*bufferPtr).length);
	BIO_free_all(bio);
	
	return output;
}



std::string BfxLibrary::GetSignature(std::string* payload)
{
	std::string digest_hex;
	char hex[2 + FOR_NULL] = { 0 };
	unsigned char * digest;

	digest = HMAC(EVP_sha384(), secretKey.c_str(), secretKey.length(), (unsigned char *)payload->c_str(), payload->length(), NULL, NULL);

	for (int i = 0; i < SHA384_DIGEST_LENGTH;i++)
	{
		sprintf(hex, "%02x", digest[i]);
		digest_hex.append(hex);
	}

	return digest_hex;
}

void BfxLibrary::AddAuthHeaders(CURL* curl, struct curl_slist **chunk, std::string* payload, std::string* signature)
{
	if (curl)
	{
		std::string apiKeyHeaderFormat = "X-BFX-APIKEY: %s";
		std::string payloadHeaderFormat = "X-BFX-PAYLOAD: %s";
		std::string signatureHeaderFormat = "X-BFX-SIGNATURE: %s";

		*chunk = curl_slist_append(*chunk, (string_format(apiKeyHeaderFormat, apiKey.c_str())).c_str());
		*chunk = curl_slist_append(*chunk, (string_format(payloadHeaderFormat, payload->c_str())).c_str());
		*chunk = curl_slist_append(*chunk, (string_format(signatureHeaderFormat, signature->c_str())).c_str());

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, *chunk);

	}
}

void BfxLibrary::InitalizeBasePayload(Document * in,char* request)
{
	in->Parse("{ }");
	requestValJson.SetString(request, strlen(request));
	in->AddMember("request", requestValJson, in->GetAllocator());
	nonce = string_format(format_long_int, GET_NONCE);
	nonceValJson.SetString(nonce.c_str(), nonce.length());
	in->AddMember("nonce", nonceValJson, in->GetAllocator());
	responseData = "";//initalize response data
}

void BfxLibrary::InitalizeWebSocketPayload(Document * in, int type)
{
	switch (type)
	{
		case new_offer:
		{
			in->Parse("[0,\"fon\",null]");
			break;
		}
		case cancel_offer:
		{
			in->Parse("[0,\"foc\",null]");
			break;
		}
	}
	BfxLibrary::webSocketRecievedData = "";
	responseData = "";
}

std::string BfxLibrary::CreateWalletPayload()
{
	Document in;
	std::string output = "";
	InitalizeBasePayload(&in,"/v1/balances");

	GetJsonString(&in, &output);

	return output;
}

std::string BfxLibrary::CreateAllCurrentOffersPayload()
{
	Document in;
	std::string output = "";
	InitalizeBasePayload(&in, "/v1/offers");

	GetJsonString(&in, &output);

	return output;
}

std::string BfxLibrary::CreateCancelOfferPayload(int offer_id)
{
	Document in;
	Value obj;
	obj.SetObject();
	std::string output = "";
	InitalizeWebSocketPayload(&in, cancel_offer);
	obj.AddMember("id", offer_id, in.GetAllocator());

	in.PushBack(obj, in.GetAllocator());

	GetJsonString(&in, &output);

	return output;
}

std::string BfxLibrary::CreateNewOfferPayload(std::string* currency, double amount, double rate, int period)
{
	Document in;
	Value obj;
	obj.SetObject();
	std::string output = "";
	double sellAmount;
	modf(amount, &sellAmount); //remove decimal
	std::string amountstr = string_format(format_decimal, sellAmount);
	std::string ratestr = string_format(format_decimal, rate);
	InitalizeWebSocketPayload(&in, new_offer);

	lendingCurrencyValJson.SetString(currency->c_str(), currency->length());
	amountValJson.SetString(amountstr.c_str(), amountstr.length());
	rateValJson.SetString(ratestr.c_str(), ratestr.length());


	obj.AddMember("type", "LIMIT", in.GetAllocator());
	obj.AddMember("symbol",lendingCurrencyValJson, in.GetAllocator());
	obj.AddMember("amount", amountValJson, in.GetAllocator());
	obj.AddMember("rate", rateValJson, in.GetAllocator());
	obj.AddMember("period", period, in.GetAllocator());
	obj.AddMember("flags", HIDDEN_FLAG, in.GetAllocator());//64 means hidden

	in.PushBack(obj, in.GetAllocator());

	GetJsonString(&in, &output);

	return output;
}

void BfxLibrary::CreateOutputPayload(CURL* curl, struct curl_slist **in_chunk)
{
	base64Payload = GetBase64(inputData);
	signature = GetSignature(&base64Payload);
	AddAuthHeaders(curl, in_chunk, &base64Payload, &signature);
}

std::string BfxLibrary::CreateWebSocketAuthPayload()
{
	Document doc;
	std::string out = "";
	unsigned long long nonce_int = GET_NONCE;
	std::string authPayload = string_format(auth_payload, nonce_int);
	signature = GetSignature(&authPayload);
	nonce = string_format(format_long_int, nonce_int);

	nonceValJson.SetString(nonce.c_str(), nonce.length());
	apiKeyValJson.SetString(apiKey.c_str(), apiKey.length());
	sigValJson.SetString(signature.c_str(), signature.length());
	authPayloadValJson.SetString(authPayload.c_str(), authPayload.length());

	doc.Parse("{ }");
	doc.AddMember("apiKey", apiKeyValJson, doc.GetAllocator());
	doc.AddMember("authSig", sigValJson, doc.GetAllocator());
	doc.AddMember("authNonce", nonceValJson, doc.GetAllocator());
	doc.AddMember("authPayload", authPayloadValJson, doc.GetAllocator());
	doc.AddMember("event", "auth", doc.GetAllocator());
	doc.AddMember("calc", 1, doc.GetAllocator());

	GetJsonString(&doc, &out);

	return out;
}

int BfxLibrary::ParseResponseData(Document * out)
{
	int cc = 0;

	ParseResult res = out->Parse< kParseStopWhenDoneFlag>(responseData.c_str());

	if (res.IsError())
		cc = MALFORMED_JSON_FROM_BFX;

	return cc;
}

int BfxLibrary::GetFundingOrderBook(Document * doc,int limitAsks)
{
	int cc = SUCCESS;
	InitCurl();
	if (curl)
	{		
		curl_easy_setopt(curl, CURLOPT_URL, (string_format(std::string(GET_LENDING_ORDERBOOK), limitAsks)).c_str());
		responseData = "";//initalize response data
		cc = curl_easy_perform(curl);

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

		if (cc == SUCCESS && response_code == HTTP_OK)
		{
			cc = ParseResponseData(doc);
		}
		else
			cc = CURL_HTTP_ERROR;

		
	}
	else
		cc = CURL_NOT_INITALIZED;

	if (cc != SUCCESS)
	{
		LOG_ERROR_CODE(cc);
	}
	

	return cc;
}

int BfxLibrary::PostFundingOffer(std::string* currency, double amount, double rate, int period, Document * out)
{
	int cc = SUCCESS;
	if (webSocket.getReadyState() == WebSocket_ReadyState_Open)
	{
		inputData = CreateNewOfferPayload(currency,amount,rate,period);
		
		cc = SendRecieveWebSocketData(&inputData, &responseData);

		if (cc == SUCCESS )
		{
			cc = ParseResponseData(out);
		}
		
		if (cc == SUCCESS)
		{
			responseString = (*out)[2].GetArray().operator[](6).GetString();
			if (responseString.compare("SUCCESS") != 0)
			{
				cc = WebSocketErrorFromBfx;
				LOG_ERROR((*out)[2].GetArray().operator[](7).GetString());
			}
		}
		
		
	}
	else
		cc = WebSocketNotConnected;

	if (cc != SUCCESS)
	{
		LOG_ERROR_CODE(cc);
	}
	return cc;
}

int BfxLibrary::PostCancelOffer(int offer_id, Document * out)
{
	int cc = SUCCESS;
	if (webSocket.getReadyState() == WebSocket_ReadyState_Open)
	{
		inputData = CreateCancelOfferPayload(offer_id);
		
		cc = SendRecieveWebSocketData(&inputData, &responseData);

		if (cc == SUCCESS)
		{
			cc = ParseResponseData(out);
		}

		if (cc == SUCCESS)
		{
			responseString = (*out)[2].GetArray().operator[](10).GetString();
			if (responseString.compare("CANCELED") != 0)
			{
				cc = WebSocketErrorFromBfx;
				LOG_ERROR(string_format(std::string("Error cancelling offer if %d"), offer_id));
			}
		}
		
		
	}
	else
		cc = WebSocketNotConnected;

	if (cc != SUCCESS)
	{
		LOG_ERROR_CODE(cc);
	}
	return cc;
}

int BfxLibrary::PostGetAllCurrentOffers(Document * out)
{
	int cc = SUCCESS;
	InitCurl();
	if (curl)
	{
		inputData = CreateAllCurrentOffersPayload();
		struct curl_slist *chunk = NULL;
		CreateOutputPayload(curl,&chunk);

		curl_easy_setopt(curl, CURLOPT_URL, GET_CURRENT_OFFERS);


		cc = curl_easy_perform(curl);

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

		if (cc == SUCCESS && response_code == HTTP_OK)
		{
			cc = ParseResponseData(out);
		}
		else
			cc = CURL_HTTP_ERROR;

		curl_slist_free_all(chunk);
	}
	else
		cc = CURL_NOT_INITALIZED;

	if (cc != SUCCESS)
	{
		LOG_ERROR_CODE(cc);
	}
	return cc;
}

int BfxLibrary::PostGetMarginAvailableFunding(Document * out)
{
	int cc = SUCCESS;
	InitCurl();
	if (curl)
	{
		inputData = CreateWalletPayload();
		struct curl_slist *chunk = NULL;
		CreateOutputPayload(curl, &chunk);

		curl_easy_setopt(curl, CURLOPT_URL, GET_WALLET_BALANCE);
		
		
		cc = curl_easy_perform(curl);

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

		if (cc == SUCCESS && response_code == HTTP_OK)
		{
			cc = ParseResponseData(out);
		}
		else
			cc = CURL_HTTP_ERROR;

		curl_slist_free_all(chunk);

	}
	else
		cc = CURL_NOT_INITALIZED;

	if (cc != SUCCESS)
	{
		LOG_ERROR_CODE(cc);
	}
	
	return cc;
}

int BfxLibrary::GetWebSocketStatus()
{
	return webSocket.getReadyState();
}


