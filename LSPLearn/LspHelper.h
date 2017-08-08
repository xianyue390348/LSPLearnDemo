#pragma once
#include "stdafx.h"

LPWSAPROTOCOL_INFO GetProvider(int &nProtocols)
{
	DWORD dwSize = 0;
	WSAPROTOCOL_INFO ProtoInfo = { 0 };
	WSAEnumProtocols(NULL, &ProtoInfo, &dwSize);
	if (dwSize <= 0) {
		return NULL;
	}
	LPWSAPROTOCOL_INFO pProtoInfos = new WSAPROTOCOL_INFO[dwSize / sizeof(WSAPROTOCOL_INFO)];
	nProtocols = WSAEnumProtocols(NULL, pProtoInfos, &dwSize);
	return pProtoInfos;
}