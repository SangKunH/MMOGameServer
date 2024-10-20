class ProtocolParser():
	def __init__(self, start_id, recv_prefix, send_prefix):
		self.recv_proto = []	# 수신 패킷 목록
		self.send_proto = [] # 송신 패킷 목록
		self.total_proto = [] # 모든 패킷 목록
		self.start_id = start_id
		self.id = start_id
		self.recv_prefix = recv_prefix
		self.send_prefix = send_prefix

	def parse_protocol(self, path):
		f = open(path, 'r')
		lines = f.readlines()

		for line in lines:
			if line.startswith('message') == False:
				continue

			proto_name = line.split()[1].upper()
			if proto_name.startswith(self.recv_prefix):
				self.recv_proto.append(Protocol(proto_name, self.id))
			elif proto_name.startswith(self.send_prefix):
				self.send_proto.append(Protocol(proto_name, self.id))
			else:
				continue

			self.total_proto.append(Protocol(proto_name, self.id))
			self.id += 1

		f.close()

class Protocol:
	def __init__(self, name, id):
		self.name = name
		self.id = id
