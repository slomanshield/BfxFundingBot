#pragma once
#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <memory>
#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>
#include <mutex>
#include <time.h>
#include <malloc.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include "logger.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/hmac.h>
#include "curl/curl.h"
#include "ixwebsocket/IXWebSocket.h"

using namespace std;
using namespace rapidjson;
using namespace ix;

#define SUCCESS 0
#define HTTP_OK 200
#define FOR_NULL 1
#define SHA384_DIGEST_LENGTH 48
#define GOOGLE_SMTP "smtps://smtp.gmail.com"
#define FROM_EMAIL "pi_2_FL@bignose.com"
#define FROM_NAME "BfxFundingBot"

typedef std::chrono::high_resolution_clock HighResolutionTime;
typedef std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::nanoseconds> HighResolutionTimePoint;
typedef std::chrono::duration<double, std::milli> DoubleMili;

#define GET_NONCE std::chrono::high_resolution_clock::now().time_since_epoch().count()

static std::string format_long_int = "%lld";
static std::string format_int = "%d";
static std::string format_string_auth = "Auth error Code: %s";
static std::string format_decimal = "%lf";
static std::string auth_payload = "AUTH%lld";

static bool TerminateApplication = false;

enum error_codes_enum
{
	CONFIG_NOT_FOUND = 1,
	INVALID_CONFIG_DATA,
	ONE_OR_MORE_CONFIG_PARAMS_MISSING,
	INVALID_CONFIG_VALUE,
	CURL_NOT_INITALIZED,
	MALFORMED_JSON_FROM_BFX,
	CURL_NO_CONNECT,
	ERROR_AUTH,
	WebSocketConnectFailure,
	WebSocketNotConnected,
	WebSocketDisonnect,
	WebSocketGetDataTimeout,
	WebSocketErrorFromBfx,
	ErrorSendingEamil,
	ErrorFindingBestRate,
	ErrorNumParams,
	CURL_HTTP_ERROR,
	NUM_ERROR_CODES
};

static char* error_codes[NUM_ERROR_CODES] =
{
	"",//since enum start at 1 we need to push everything down by 1
	"Config File Not Found",
	"Invalid Config Data",
	"One or more config files missing",
	"Invalid Config Value",
	"Curl Not Initalized",
	"Malformed JSON from Bitfinex",
	"Curl could not connect to Bitfinex",
	"Error websocket Auth",
	"Error connecting to websocket",
	"Error websocket not connected",
	"Disconnected from WebSocket",
	"Timeout Getting Data From WebSocket",
	"Error From Bitfinex Websocket Data",
	"Error Sending Email",
	"Error Finding Best Rate",
	"Not enough Parameters, need full Config path",
	"Error Getting data from rest call"
};

template<typename ... Args>
static std::string string_format(const std::string& format, Args ... args)
{
	size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	char* buffer = new char[size];
	std::string out;
	memset(buffer, 0, size);
	snprintf(buffer, size, format.c_str(), args ...);
	out = buffer;
	delete[] buffer;
	return out;
}

static void LOG_ERROR_CODE(int cc)
{
	std::string error = "%s";
	std::string genericError = "ErrorCode is :%d";
	if (cc >= NUM_ERROR_CODES)
		LOG_ERROR(string_format(genericError, cc));
	else
		LOG_ERROR(string_format(error, error_codes[cc]));
}

static void GetJsonString(Document* doc, std::string* out)
{
	rapidjson::StringBuffer buffer;

	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc->Accept(writer);

	out->append(buffer.GetString());

}

static std::string dateTimeNow()
{
	const int RFC5322_TIME_LEN = 32;
	time_t t;
	struct tm *tm;

	std::string ret;
	ret.resize(RFC5322_TIME_LEN);

	time(&t);
	tm = localtime(&t);

	strftime(&ret[0], RFC5322_TIME_LEN, "%a, %d %b %Y %H:%M:%S %z", tm);

	return ret;
}

static int GetWholeNumAmount(double n)
{
	std::string numString = string_format(std::string("%llf"), n);

	return numString.find(".");
	
}

static double convert_365_rate_to_decimal(double n)
{
	return ((n / 365) / 100);
}