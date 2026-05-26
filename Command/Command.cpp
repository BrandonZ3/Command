#include <WinSock2.h>
#include <WS2tcpip.h>
#include "../../Broken/Lib/Lib.cpp"

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Ws2_32.lib")

bool kill = false;
sockaddr_in server = { 0 };
SOCKET client;

void StandardCommands(Command* comm)
{
	if ((StringCompareCaseInsensitive(comm->command, "connect") || StringCompareCaseInsensitive(comm->command, "c")) && comm->arguments->count >= 2)
	{
		bool ipFound = false;
		bool portFound = false;

		server.sin_family = AF_INET;

		for (size_t i = 0; i < comm->arguments->count; i++)
		{
			if (IsIp(comm->arguments->items[i]))
			{
				server.sin_addr.s_addr = IPToDwordNetwork(comm->arguments->items[i]);
				//inet_pton(AF_INET, comm->arguments->items[i], &server.sin_addr);
				ipFound = true;
			}

			if (IsNumeric(comm->arguments->items[i]))
			{
				unsigned long port = StringToUnsignedDword(comm->arguments->items[i]);
				if (port <= 65535)
				{
					server.sin_port = htons(port);
					portFound = true;
				}
			}
		}

		if (ipFound && portFound)
		{
			int res = connect(client, (const sockaddr*)&server, sizeof(server));

			if (res == 0)
			{
				printf("Connected\r\n");
			}
			else
			{
				printf("Failed To Connect\r\n");
			}
		}
	}

	if ((StringCompareCaseInsensitive(comm->command, "disconnect") || StringCompareCaseInsensitive(comm->command, "d")))
	{
		closesocket(client);
		client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}

	if ((StringCompareCaseInsensitive(comm->command, "send") || StringCompareCaseInsensitive(comm->command, "s")) && comm->flags->count > 0)
	{
		for (size_t i = 0; i < comm->flags->count; i++)
		{
			if (StringCompare(comm->flags->items[i]->key, "-I") && comm->flags->items[i]->value != NULL)
			{
				Buffer* buf = ReadFile(comm->flags->items[i]->value, "rb", 16000000);

				int sent = send(client, (char*)buf->buffer, buf->count, 0);

				if (sent == buf->count)
				{
					printf("Data Sent\r\n");
				}
				else
				{
					printf("Data Send Failed!\r\n");
				}

				delete buf;
			}

			if (StringCompare(comm->flags->items[i]->key, "-T") && comm->flags->items[i]->value != NULL)
			{
				const char* buf = comm->flags->items[i]->value;
				//size_t length = strlen(buf);

				char* newS = ConvertBareText(buf);
				size_t length = strlen(newS);

				//int sent = send(client, buf, length, 0);
				int sent = send(client, newS, length, 0);

				if (sent == length)
				{
					printf("Data Sent\r\n");
				}
				else
				{
					printf("Data Send Failed!\r\n");
				}

				free(newS);
			}

			if (StringCompare(comm->flags->items[i]->key, "-X") && comm->flags->items[i]->value != NULL)
			{
				Buffer* buf = BufferFromHexString(comm->flags->items[i]->value);

				int sent = send(client, (char*)buf->buffer, buf->count, 0);

				if (sent == buf->count)
				{
					printf("Data Sent\r\n");
				}
				else
				{
					printf("Data Send Failed!\r\n");
				}

				delete buf;
			}
		}
	}

	if ((StringCompareCaseInsensitive(comm->command, "tohex") || StringCompareCaseInsensitive(comm->command, "th")) && comm->flags->count > 0)
	{
		for (size_t i = 0; i < comm->flags->count; i++)
		{
			if (StringCompare(comm->flags->items[i]->key, "-T") && comm->flags->items[i]->value != NULL)
			{
				const char* buf = comm->flags->items[i]->value;
				//size_t length = strlen(buf);

				char* newS = ConvertBareText(buf);

				char* converted = StringToHexString(newS);

				printf("%s\r\n", converted);

				free(converted);
				free(newS);
			}
		}
	}

	if ((StringCompareCaseInsensitive(comm->command, "hextofloat") || StringCompareCaseInsensitive(comm->command, "htf")))
	{
		if (comm->arguments->count > 0)
		{
			int length = strlen(comm->arguments->items[0]);
			if (length == 8 || length == 16)
			{
				Buffer* buf = BufferFromHexString(comm->arguments->items[0]);

				if (length == 8)
				{
					uint32_t value = buf->buffer[0] | buf->buffer[1] << 8 | buf->buffer[2] << 16 | buf->buffer[3] << 24;
					float* nval = (float*)&value;
					printf("float32:%f\r\n", *nval);
				}
				else
				{
					uint64_t value = buf->buffer[0] | buf->buffer[1] << 8 | buf->buffer[2] << 16 | buf->buffer[3] << 24 | buf->buffer[4] << 32 | buf->buffer[5] << 40 | buf->buffer[6] << 48 | buf->buffer[7] << 56;
					double* nval = (double*)&value;
					printf("float64:%f\r\n", *nval);
				}
				delete buf;
			}
			else
			{
				printf("Hex Float Needs 8 or 16 characters!\r\n");
			}
		}
	}

	if ((StringCompareCaseInsensitive(comm->command, "receive") || StringCompareCaseInsensitive(comm->command, "r")))
	{
		char buf[2048] = { 0 };
		char* filepath = NULL;
		bool body = false;
		bool http = false;
		bool headers = false;
		bool append = false;
		bool output = false;

		for (size_t i = 0; i < comm->flags->count; i++)
		{
			if (StringCompare(comm->flags->items[i]->key, "-O") && comm->flags->items[i]->value != NULL && !append)
			{
				filepath = comm->flags->items[i]->value;
				output = true;
			}

			if (StringCompare(comm->flags->items[i]->key, "-A") && comm->flags->items[i]->value != NULL && !output)
			{
				filepath = comm->flags->items[i]->value;
				append = true;
			}

			if (StringCompare(comm->flags->items[i]->key, "-b") && !headers)
			{
				body = true;
			}

			if (StringCompare(comm->flags->items[i]->key, "-h") && !body)
			{
				headers = true;
			}

			if (StringCompare(comm->flags->items[i]->key, "-H"))
			{
				http = true;
			}
		}

		size_t recLength = recv(client, buf, 2048, 0);

		if (filepath)
		{
			Buffer* buffer = new Buffer(2048);
			if (http)
			{
				if (headers)
				{
					const char* test = "aabba";
					int ind = FirstIndexOf(test, "bb");

					int index = FirstIndexOf(buf, "\r\n\r\n");
					buffer->Add(buf, index);
				}
				else if (body)
				{
					int index = FirstIndexOf(buf, "\r\n\r\n");
					buffer->Add(buf + (index + 4), recLength - (index + 4));
				}
				else
				{
					buffer->Add(buf, recLength);
				}
			}
			else
			{
				buffer->Add(buf, recLength);
			}
			if (output)
				WriteFile(filepath, buffer);
			if (append)
				AppendFile(filepath, buffer);
			delete buffer;
		}

		printf("%s\r\n", buf);
	}

	if ((StringCompareCaseInsensitive(comm->command, "help") || StringCompareCaseInsensitive(comm->command, "h")))
	{
		const char* information =
			R"(
Commands are case insensitive and Flags are case sensitive.
Format: %%Command%% [%%...ARGUMENTS%%] [-%%...FLAG%%] [-%%...FLAGS%% %%...FLAGVALUES%%] 

connect | c - connect to a tcp socket.
	Arguments: 
		Ip Address - Specify an IP address with this format (###.###.###.###). eg. \"192.168.0.4\"
		Port - Specify an port with this format (#####). eg. \"80\"
	Flags: 
		None

disconnect | d - disconnect from tcp connection.
	Arguments: 
		None
	Flags: 
		None

execute | e - Execute a script using the standard commands, which exclude this command from a file.
	Arguments: 
		Full file path - File containing the script. eg. \"C:\\Users\\User\\Desktop\\Test.txt\"
	Flags: 
		-R %%Count%% - Repeats the script for the \"count\" specified.

help | h - Displays this help message.
	Arguments: 
		None
	Flags: 
		None

hextofloat | htf - Convert HEX string to floating point value.
	Arguments: 
		HEX String - Hex string in this case must be 8 or 16 characters to fit 32/64-bit floating point conversions. eg. \"4554ABDC\"
	Flags: 
		None

quit | q - Ends this programs life.
	Arguments: 
		None
	Flags: 
		None

receive | r - receive data over the network.
	Arguments: 
		None
	Flags: 
		-O %%Full File Path%% - Outputs message to file specified. eg. -O \"C:\\Users\\User\\Desktop\\Test.txt\"
		-A %%Full File Path%% - Appends message to file specified. eg. -A \"C:\\Users\\User\\Desktop\\Test.txt\"
		-H - specifies that the message is HTTP, this flag is for combining with the next 2 flags.
		-h - can only be used with H flag to only use the headers of the HTTP message.
		-b - can only be used with H flag to only use the body of the HTTP message.

send | s - send data over the network.
	Arguments: 
		None
	Flags: 
		-I %%Full File Path%% - Sends the raw data after reading the file. eg. -I \"C:\\Users\\User\\Desktop\\Test.txt\"
		-T %%Text String%% - Sends the data after converting from string to binary. eg. -T \"Hi this is a message.\"
		-X %%HEX String%% - Sends the data after converting from hex string to binary. eg. -X \"4554ABDC\"

tohex | th - Convert from text string to hex.
	Arguments: 
		None
	Flags:
		-T %%Text String%% - Prints the data as hexstring after converting from string to binary. eg. -T \"Hi this is a message.\"
						)";

		printf("%s\r\n", information);
	}

	if ((StringCompareCaseInsensitive(comm->command, "quit") || StringCompareCaseInsensitive(comm->command, "q")))
	{
		kill = true;
	}
}

void main()
{
	WSADATA Dat;
	int res = WSAStartup(0x0202, &Dat);
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	while (true)
	{
		char buf[1024] = { 0 };
		fgets(buf, 1024, stdin);

		if (strlen(buf) != 1)
		{
			Command* comm = new Command(buf);

			if (comm->command)
			{
				if ((StringCompareCaseInsensitive(comm->command, "execute") || StringCompareCaseInsensitive(comm->command, "e")))
				{
					size_t repeat = 1;

					for (size_t i = 0; i < comm->flags->count; i++)
					{
						if (StringCompare(comm->flags->items[i]->key, "-R") && comm->flags->items[i]->value != NULL)
						{
							repeat = StringToUnsignedDword((const char*)(comm->flags->items[i]->value));
						}
					}

					if (comm->arguments->count == 1)
					{
						Buffer* file = ReadFile(comm->arguments->items[0], "rb", 16000000);
						file->Add(0);

						StringArray* stringArr = Splitter((char*)file->buffer, "\r\n");


						while (repeat > 0)
						{
							repeat--;
							for (size_t i = 0; i < stringArr->count; i++)
							{
								printf("%s\r\n", stringArr->items[i]);
								if (strlen(stringArr->items[i]) >= 1)
								{
									Command* newCom = new Command(stringArr->items[i]);

									StandardCommands(newCom);

									delete newCom;
								}

							}
						}

						stringArr->FreeEverything();
						delete stringArr;

						delete file;
					}
				}
				else
					StandardCommands(comm);


				if (kill)
				{
					delete comm;
					break;
				}
			}

			delete comm;
		}
	}
	WSACleanup();
}