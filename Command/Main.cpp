#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <stdio.h>
#include "../../Library/Systems/Command.h"
#include "../../Library/Base/Strings.h"
#include "../../Library/IO/Files.h"

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Ws2_32.lib")

bool kill = false;
sockaddr_in server = { 0 };
SOCKET client;

struct Escapers
{
	size_t started;
	size_t ended;
};

Escapers CountEscaped(const char* string)
{
	Escapers esc = { 0 };

	size_t stringLength = strlen(string);

	for (size_t i = 0; i < stringLength; i++)
	{
		bool found = false;

		if (
			(string[i] == '\\' && string[i + 1] == '\\') ||
			(string[i] == '\\' && string[i + 1] == 'r') ||
			(string[i] == '\\' && string[i + 1] == 'n') ||
			(string[i] == '\\' && string[i + 1] == 't') ||
			(string[i] == '\\' && string[i + 1] == '\'') ||
			(string[i] == '\\' && string[i + 1] == '\"')
			)
		{
			esc.ended += 1;
			esc.started += 1;
		}
		else if (string[i] == '\\')
		{
			esc.started += 1;
		}
	}

	return esc;
}

char* ConvertBareText(const char* string)
{
	size_t stringLen = strlen(string);
	Escapers esc = CountEscaped(string);

	if (esc.started == esc.ended)
	{
		size_t newLen = (stringLen - (esc.ended));

		char* output = (char*)malloc(newLen + 1);
		output[newLen] = 0;
		size_t pos = 0;

		for (size_t i = 0; i < stringLen; i++)
		{
			if (string[i] == '\\' && string[i + 1] == '\\')
			{
				output[pos] = '\\';
				pos++;
				i++;
				continue;
			}

			if (string[i] == '\\' && string[i + 1] == 't')
			{
				output[pos] = '\t';
				pos++;
				i++;
				continue;
			}

			if (string[i] == '\\' && string[i + 1] == '\'')
			{
				output[pos] = '\'';
				pos++;
				i++;
				continue;
			}

			if (string[i] == '\\' && string[i + 1] == '\"')
			{
				output[pos] = '\"';
				pos++;
				i++;
				continue;
			}

			if (string[i] == '\\' && string[i + 1] == 'r')
			{
				output[pos] = '\r';
				pos++;
				i++;
				continue;
			}


			if (string[i] == '\\' && string[i + 1] == 'n')
			{
				output[pos] = '\n';
				pos++;
				i++;
				continue;
			}

			output[pos] = string[i];
			pos++;
		}

		return output;
	}
	return NULL;
}


void StandardCommands(Command* comm)
{
	if ((Strings::CompareCaseInsensitive(comm->command, "connect") || Strings::CompareCaseInsensitive(comm->command, "c")) && comm->arguments->count >= 2)
	{
		bool ipFound = false;
		bool portFound = false;

		server.sin_family = AF_INET;

		for (size_t i = 0; i < comm->arguments->count; i++)
		{
			if (Strings::IsIp((const char*)comm->arguments->items[i]))
			{
				server.sin_addr.s_addr = Strings::IPToDwordNetwork((const char*)comm->arguments->items[i]);
				ipFound = true;
			}

			if (Strings::IsNumeric((const char*)comm->arguments->items[i]))
			{
				unsigned long port = Strings::StringToUnsignedDword((const char*)comm->arguments->items[i]);
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

	if ((Strings::CompareCaseInsensitive(comm->command, "disconnect") || Strings::CompareCaseInsensitive(comm->command, "d")))
	{
		closesocket(client);
		client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}

	if ((Strings::CompareCaseInsensitive(comm->command, "send") || Strings::CompareCaseInsensitive(comm->command, "s")) && comm->flags->count > 0)
	{
		for (size_t i = 0; i < comm->flags->count; i++)
		{
			if (Strings::Compare(comm->flags->items[i]->key, "-I") && comm->flags->items[i]->value != NULL)
			{
				DBuffer* buf = Files::ReadFile(comm->flags->items[i]->value, "rb", 16000000);

				int sent = send(client, (char*)buf->DataPointer(0), buf->count, 0);

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

			if (Strings::Compare(comm->flags->items[i]->key, "-T") && comm->flags->items[i]->value != NULL)
			{
				const char* buf = comm->flags->items[i]->value;

				char* newS = ConvertBareText(buf);
				size_t length = strlen(newS);

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

			if (Strings::Compare(comm->flags->items[i]->key, "-X") && comm->flags->items[i]->value != NULL)
			{
				DBuffer* buf = Strings::BufferFromHexString((const char*)comm->flags->items[i]->value);

				int sent = send(client, (char*)buf->DataPointer(0), buf->count, 0);

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

	if ((Strings::CompareCaseInsensitive(comm->command, "tohex") || Strings::CompareCaseInsensitive(comm->command, "th")) && comm->flags->count > 0)
	{
		for (size_t i = 0; i < comm->flags->count; i++)
		{
			if (Strings::Compare((const char*)comm->flags->items[i]->key, "-T") && comm->flags->items[i]->value != NULL)
			{
				const char* buf = comm->flags->items[i]->value;

				char* newS = ConvertBareText(buf);

				char* converted = Strings::StringToHexString(newS);

				printf("%s\r\n", converted);

				free(converted);
				free(newS);
			}
		}
	}

	if ((Strings::CompareCaseInsensitive(comm->command, "hextofloat") || Strings::CompareCaseInsensitive(comm->command, "htf")))
	{
		if (comm->arguments->count > 0)
		{
			int length = strlen((const char*)comm->arguments->items[0]);
			if (length == 8 || length == 16)
			{
				DBuffer* buf = Strings::BufferFromHexString((const char*)comm->arguments->items[0]);

				unsigned char* buffer = buf->DataPointer(0);

				if (length == 8)
				{
					uint32_t value = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
					float* nval = (float*)&value;
					printf("float32:%f\r\n", *nval);
				}
				else
				{
					uint64_t value = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24 | buffer[4] << 32 | buffer[5] << 40 | buffer[6] << 48 | buffer[7] << 56;
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

	if ((Strings::CompareCaseInsensitive(comm->command, "receive") || Strings::CompareCaseInsensitive(comm->command, "r")))
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
			if (Strings::Compare(comm->flags->items[i]->key, "-O") && comm->flags->items[i]->value != NULL && !append)
			{
				filepath = comm->flags->items[i]->value;
				output = true;
			}

			if (Strings::Compare(comm->flags->items[i]->key, "-A") && comm->flags->items[i]->value != NULL && !output)
			{
				filepath = comm->flags->items[i]->value;
				append = true;
			}

			if (Strings::Compare(comm->flags->items[i]->key, "-b") && !headers)
			{
				body = true;
			}

			if (Strings::Compare(comm->flags->items[i]->key, "-h") && !body)
			{
				headers = true;
			}

			if (Strings::Compare(comm->flags->items[i]->key, "-H"))
			{
				http = true;
			}
		}

		size_t recLength = recv(client, buf, 2048, 0);

		if (filepath)
		{
			DBuffer* buffer = new DBuffer(2048);
			if (http)
			{
				if (headers)
				{
					const char* test = "aabba";
					int ind = Strings::IndexOf(test, "bb", 1);

					int index = Strings::IndexOf(buf, "\r\n\r\n", 1);
					buffer->Add((unsigned char*)buf, index);
				}
				else if (body)
				{
					int index = Strings::IndexOf(buf, "\r\n\r\n", 1);
					buffer->Add((unsigned char*)(buf + (index + 4)), recLength - (index + 4));
				}
				else
				{
					buffer->Add((unsigned char*)buf, recLength);
				}
			}
			else
			{
				buffer->Add((unsigned char*)buf, recLength);
			}
			if (output)
				Files::WriteFile(filepath, buffer);
			if (append)
				Files::AppendFile(filepath, buffer);
			delete buffer;
		}

		printf("%s\r\n", buf);
	}

	if ((Strings::CompareCaseInsensitive(comm->command, "help") || Strings::CompareCaseInsensitive(comm->command, "h")))
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

	if ((Strings::CompareCaseInsensitive(comm->command, "quit") || Strings::CompareCaseInsensitive(comm->command, "q")))
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
				if ((Strings::CompareCaseInsensitive(comm->command, "execute") || Strings::CompareCaseInsensitive(comm->command, "e")))
				{
					size_t repeat = 1;

					for (size_t i = 0; i < comm->flags->count; i++)
					{
						if (Strings::Compare(comm->flags->items[i]->key, "-R") && comm->flags->items[i]->value != NULL)
						{
							repeat = Strings::StringToUnsignedDword((const char*)(comm->flags->items[i]->value));
						}
					}

					if (comm->arguments->count == 1)
					{
						DBuffer* file = Files::ReadFile((char*)comm->arguments->items[0], "rb", 16000000);
						file->Add((uint8_t)0);

						PointerList* stringArr = PointerList::SplitString((char*)file->DataPointer(0), "\r\n");


						while (repeat > 0)
						{
							repeat--;
							for (size_t i = 0; i < stringArr->count; i++)
							{
								printf("%s\r\n", stringArr->items[i]);
								if (strlen((const char*)stringArr->items[i]) >= 1)
								{
									Command* newCom = new Command((char*)stringArr->items[i]);

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