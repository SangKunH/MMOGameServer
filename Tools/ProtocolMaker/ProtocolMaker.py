import argparse
import jinja2
import ProtocolParser

def main():

	arg_parser = argparse.ArgumentParser(description = 'ProtocolMaker')
	arg_parser.add_argument('--path', type=str, default='C:/willyhan/WillySJHan/Game/Resource/protoc/bin/Protocol.proto', help='protocol path')
	arg_parser.add_argument('--output', type=str, default='TestProtocolManager', help='output file')
	arg_parser.add_argument('--recv', type=str, default='UC_', help='recv convention')
	arg_parser.add_argument('--send', type=str, default='MS_', help='send convention')
	args = arg_parser.parse_args()

	parser = ProtocolParser.ProtocolParser(1000, args.recv, args.send)
	parser.parse_protocol(args.path)
	file_loader = jinja2.FileSystemLoader('Templates')
	env = jinja2.Environment(loader=file_loader)

	template = env.get_template('ProtocolManager.h')
	output = template.render(parser=parser, output=args.output)

	f = open(args.output+'.h', 'w+')
	f.write(output)
	f.close()

	print(output)
	return

if __name__ == '__main__':
	main()
