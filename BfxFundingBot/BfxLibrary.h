#pragma once
#include "global.h"

#define GET_LENDING_ORDERBOOK "https://api.bitfinex.com/v1/lendbook/usd?limit_bids=0&limit_asks=%d"
#define GET_WALLET_BALANCE "https://api.bitfinex.com/v1/balances"
#define GET_CURRENT_OFFERS "https://api.bitfinex.com/v1/offers"
#define POST_NEW_OFFER "https://api.bitfinex.com/v2/offer/new"
#define CANCEL_OFFER "https://api.bitfinex.com/v2/offer/cancel"
#define WebSocketURL "wss://api.bitfinex.com/ws/2"

#define HIDDEN_FLAG 64
#define MINIMUM_AMOUNT 50

static size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {
	data->append((char*)ptr, size * nmemb);
	return size * nmemb;
}
class BfxLibrary
{
	public:
		BfxLibrary();
		~BfxLibrary();
		int ConnectWebSocket();
		int SendAuthWebSocket();
		int DisconnetWebSocket();
		void InitKeys(char* apiKey, char* secretKey);
		int GetFundingOrderBook(Document* doc,int limitAsks);
		int PostFundingOffer(std::string* currency,double amount,double rate, int period, Document* out);
		int PostCancelOffer(int offer_id,Document* out);
		int PostGetAllCurrentOffers(Document* out);
		int PostGetMarginAvailableFunding(Document* out);
		int GetWebSocketStatus();
	private:
		std::string apiKey;
		std::string secretKey;
		std::string GetSignature(std::string* payload);
		std::string GetBase64(std::string input);
		void InitalizeBasePayload(Document* in, char* request);
		void InitalizeWebSocketPayload(Document* in, int type);
		int ParseResponseData(Document* out);
		int SendRecieveWebSocketData(std::string* in, std::string* out);
		void InitCurl();

		std::string CreateWalletPayload();
		std::string CreateAllCurrentOffersPayload();
		std::string CreateCancelOfferPayload(int offer_id);
		std::string CreateNewOfferPayload(std::string* currency, double amount, double rate, int period);
		std::string CreateWebSocketAuthPayload();
		void AddAuthHeaders(CURL* curl, struct curl_slist **chunk, std::string* payload, std::string* signature);
		void CreateOutputPayload(CURL* curl, struct curl_slist **in_chunk);
		
		CURL* curl;
		std::string responseData;
		std::string inputData;
		std::string base64Payload;
		std::string signature;
		std::string nonce;
		std::string authStatus;
		long response_code;
		Value nonceValJson;
		Value requestValJson;
		Value apiKeyValJson;
		Value sigValJson;
		Value authPayloadValJson;
		Value lendingCurrencyValJson;
		Value amountValJson;
		Value rateValJson;

		ix::WebSocket webSocket;
		static bool dataReceived;
		static std::string webSocketRecievedData;
		std::string responseString;
		static void recieveCallBack(ix::WebSocketMessageType SocketMessageType,
			const std::string& str,
			size_t wireSize,
			const WebSocketErrorInfo& error,
			const WebSocketOpenInfo& closeInfo,
			const WebSocketCloseInfo& headers);
		int WaitForData(double timeoutMilliseconds);
		enum{new_offer,cancel_offer};
};