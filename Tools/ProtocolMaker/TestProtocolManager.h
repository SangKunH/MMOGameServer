#pragma once
#include "Protocol.pb.h"

using FProtocolManager = std::function<bool(std::shared_ptr<ProtocolSession>&, BYTE*, int32_t)>;
extern FProtocolManager g_protocolManager[UINT16_MAX];

enum : uint16_t
{
	UC_TEST = 1000,
	UC_MOVE = 1001,
	MS_TEST = 1002,
	MS_LOGIN = 1003,
};

bool Manage_INVALID(std::shared_ptr<ProtocolSession>& session, BYTE* buffer, int32_t len);
bool Manage_UC_TEST(std::shared_ptr<ProtocolSession>& session, Protocol::UC_TEST& proto);
bool Manage_UC_MOVE(std::shared_ptr<ProtocolSession>& session, Protocol::UC_MOVE& proto);

class TestProtocolManager
{
public:
	static void Init()
	{
		for (int32_t i = 0; i < UINT16_MAX; i++)
			g_protocolManager[i] = Manage_INVALID;
		g_protocolManager[UC_TEST] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::UC_TEST>(Manage_UC_TEST, session, buffer, len); };
		g_protocolManager[UC_MOVE] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::UC_MOVE>(Manage_UC_MOVE, session, buffer, len); };
	}

	static bool ManageProtocol(std::shared_ptr<ProtocolSession>& session, BYTE* buffer, int32_t len)
	{
		ProtocolHeader* header = reinterpret_cast<ProtocolHeader*>(buffer);
		return g_protocolManager[header->id](session, buffer, len);
	}
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_TEST& proto) { return MakeSendBuffer(proto, MS_TEST); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_LOGIN& proto) { return MakeSendBuffer(proto, MS_LOGIN); }

private:
	template<typename ProtocolType, typename FManage>
	static bool ManageProtocol(FManage f_manage, std::shared_ptr<ProtocolSession>& session, BYTE* buffer, int32_t len)
	{
		ProtocolType proto;
		if (proto.ParseFromArray(buffer + sizeof(ProtocolHeader), len - sizeof(ProtocolHeader)) == false)
			return false;

		return f_manage(session, proto);
	}

	template<typename T>
	static std::shared_ptr<SendBuffer> MakeSendBuffer(T& proto, uint16_t protoId)
	{
		const uint16_t dataSize = static_cast<uint16_t>(proto.ByteSizeLong());
		const uint16_t packetSize = dataSize + sizeof(ProtocolHeader);

		std::shared_ptr<SendBuffer> sendBuffer = std::make_shared<SendBuffer>(packetSize);
		ProtocolHeader* header = reinterpret_cast<ProtocolHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = protoId;
		SJ_ASSERT(proto.SerializeToArray(&header[1], dataSize));
		sendBuffer->SetWriteSize(packetSize);

		return sendBuffer;
	}
};
