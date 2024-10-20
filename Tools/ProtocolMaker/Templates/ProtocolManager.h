#pragma once
#include "Protocol.pb.h"

using FProtocolManager = std::function<bool(std::shared_ptr<ProtocolSession>&, BYTE*, int32_t)>;
extern FProtocolManager g_protocolManager[UINT16_MAX];

enum : uint16_t
{
{%- for proto in parser.total_proto %}
	{{proto.name}} = {{proto.id}},
{%- endfor %}
};

bool Manage_INVALID(std::shared_ptr<ProtocolSession>& session, BYTE* buffer, int32_t len);

{%- for proto in parser.recv_proto %}
bool Manage_{{proto.name}}(std::shared_ptr<ProtocolSession>& session, Protocol::{{proto.name}}& proto);
{%- endfor %}

class {{output}}
{
public:
	static void Init()
	{
		for (int32_t i = 0; i < UINT16_MAX; i++)
			g_protocolManager[i] = Manage_INVALID;

{%- for proto in parser.recv_proto %}
		g_protocolManager[{{proto.name}}] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::{{proto.name}}>(Manage_{{proto.name}}, session, buffer, len); };
{%- endfor %}
	}

	static bool ManageProtocol(std::shared_ptr<ProtocolSession>& session, BYTE* buffer, int32_t len)
	{
		ProtocolHeader* header = reinterpret_cast<ProtocolHeader*>(buffer);
		return g_protocolManager[header->id](session, buffer, len);
	}

{%- for proto in parser.send_proto %}
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::{{proto.name}}& proto) { return MakeSendBuffer(proto, {{proto.name}}); }
{%- endfor %}

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

