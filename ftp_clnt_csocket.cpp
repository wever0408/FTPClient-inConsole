// ftp_clnt_csocket.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "ftp_clnt_csocket.h"
#include <afxsock.h>
#include <string>
#include <sys/stat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

enum MODE
{
	ACTIVE, PASSIVE
};

void replylogcode(int code);
int sendPORT(CSocket *Client, CSocket *Transfer);
int getServerPort(char *msg);
void logIn(CSocket *Client, char *info);
void listDIR(CSocket *ClientSocket, char *dir, MODE mode);
void nameListLS(CSocket *ClientSocket, char *ls, MODE mode);
int nameListWithLessMessageLS(CSocket *ClientSocket, char *ls);
void downloadFileGET(CSocket *ClientSocket, char *file, MODE mode);
void downloadFilesMGET(CSocket *ClientSocket, char *file, MODE mode);
void UploadFilePUT(CSocket *ClientSocket, char *file, MODE mode);
void UploadFilesMPUT(CSocket *ClientSocket, char *file, MODE mode);
void localChangeDirectoryLCD(char *info);
void changeDirectoryCD(CSocket *ClientSocket, char *cd);
void fileDELETE(CSocket *ClientSocket, char *file);
void filesMDELETE(CSocket *ClientSocket, char *file);
void makeDirectoryMKDIR(CSocket *ClientSocket, char *folder);
void removeDirectoryRMDIR(CSocket *ClientSocket, char *folder);
void PWD(CSocket *ClientSocket);
int quitOrExit(CSocket *ClientSocket);

int port = 3500;
char IPClient[] = "192,168,85,2";
char IPServer[] = "192.168.85.3";
char *lcd = "C:\\Users\\kylenguyen\\Client";
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;
	MODE mode = MODE::ACTIVE;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: code your application's behavior here.
			// Khoi tao thu vien Socket
			if (AfxSocketInit() == FALSE)
			{
				cout << "Khong the khoi tao Socket Libraray";
				return FALSE;
			}
			// Tao socket dau tien
			CSocket ClientSocket;
			ClientSocket.Create();

			// Ket noi den Server
			if (ClientSocket.Connect(_T("192.168.85.3"), 21) != 0)
			{
				cout << "Ket noi toi Server thanh cong !!!" << endl << endl;
			}
			else
				return FALSE;

			char buf[BUFSIZ + 1];
			int tmpres;
			/*
			Connection Establishment
			120
			220
			220
			421
			Login
			USER
			230
			530
			500, 501, 421
			331, 332
			PASS
			230
			202
			530
			500, 501, 503, 421
			332
			*/
			char * str;
			int codeftp;
			printf("Connection established, waiting for welcome message...\n");
			//How to know the end of welcome message: http://stackoverflow.com/questions/13082538/how-to-know-the-end-of-ftp-welcome-message
			memset(buf, 0, sizeof buf);
			while ((tmpres = ClientSocket.Receive(buf, BUFSIZ, 0)) > 0) {
				sscanf(buf, "%d", &codeftp);
				printf("%s", buf);
				if (codeftp != 220) //120, 240, 421: something wrong
				{
					replylogcode(codeftp);
					exit(1);
				}

				str = strstr(buf, "220");//Why ???
				if (str != NULL) {
					break;
				}
				memset(buf, 0, tmpres);
			}
			//Send Username
			char info[50];
			printf("Name (%s): ", "192.168.85.2");
			memset(buf, 0, sizeof buf);
			scanf("%s", info);

			sprintf(buf, "USER %s\r\n", info);
			tmpres = ClientSocket.Send(buf, strlen(buf), 0);

			memset(buf, 0, sizeof buf);
			tmpres = ClientSocket.Receive(buf, BUFSIZ, 0);

			sscanf(buf, "%d", &codeftp);
			if (codeftp != 331)
			{
				replylogcode(codeftp);
			}
			else
			{
				printf("%s", buf);
				//Send Password
				memset(info, 0, sizeof info);
				printf("Password: ");
				memset(buf, 0, sizeof buf);
				scanf("%s", info);

				sprintf(buf, "PASS %s\r\n", info);
				tmpres = ClientSocket.Send(buf, strlen(buf), 0);

				memset(buf, 0, sizeof buf);
				tmpres = ClientSocket.Receive(buf, BUFSIZ, 0);

				sscanf(buf, "%d", &codeftp);
				if (codeftp != 230)
				{
					replylogcode(codeftp);
				}
				else printf("%s", buf);
			}
			char cmd[50];
			char *tok;
			do
			{
				char tmp[50];
				fflush(stdin);
				printf("fpt> ");
				gets(tmp);
				strcpy(cmd, tmp);
				tok = strtok(tmp, " ");
				if (stricmp(tok, "user") == 0)
				{
					logIn(&ClientSocket, cmd);
				}
				else if (stricmp(tok, "dir") == 0)
				{
					listDIR(&ClientSocket, cmd, mode);
				}
				else if (stricmp(tok, "ls") == 0)
				{
					nameListLS(&ClientSocket, cmd, mode);
				}
				else if (stricmp(tok, "get") == 0)
				{
					downloadFileGET(&ClientSocket, cmd, mode);
				}
				else if (stricmp(tok, "mget") == 0)
				{
					downloadFilesMGET(&ClientSocket, cmd, mode);
				}
				else if (stricmp(tok, "put") == 0)
				{
					UploadFilePUT(&ClientSocket, cmd, mode);
				}
				else if (stricmp(tok, "mput") == 0)
				{
					UploadFilesMPUT(&ClientSocket, cmd, mode);
				}
				else if (stricmp(tok, "lcd") == 0)
				{
					localChangeDirectoryLCD(cmd);
				}
				else if (stricmp(tok, "cd") == 0)
				{
					changeDirectoryCD(&ClientSocket, cmd);
				}
				else if (stricmp(tok, "delete") == 0)
				{
					fileDELETE(&ClientSocket, cmd);
				}
				else if (stricmp(tok, "mdelete") == 0)
				{
					filesMDELETE(&ClientSocket, cmd);
				}
				else if (stricmp(tok, "mkdir") == 0)
				{
					makeDirectoryMKDIR(&ClientSocket, cmd);
				}
				else if (stricmp(tok, "rmdir") == 0)
				{
					removeDirectoryRMDIR(&ClientSocket, cmd);
				}
				else if (stricmp(tok, "pwd") == 0)
				{
					PWD(&ClientSocket);
				}
				else if (stricmp(tok, "passive") == 0)
				{
					mode = MODE::PASSIVE;
				}
				else if (stricmp(tok, "active") == 0)
				{
					mode = MODE::ACTIVE;
				}
				else if (stricmp(tok, "exit") == 0 || stricmp(tok, "quit") == 0)
				{
					if(quitOrExit(&ClientSocket) == 1)
					{
						break;
					}
				}

			} while (1);
			ClientSocket.Close();
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}


int sendPORT(CSocket *Client, CSocket *Transfer)
{

	char msg[BUFSIZ+1];
	int codeftp;
	int div = port / 256;
	int r = port % 256;
	sprintf(msg, "PORT %s,%d,%d\r\n", IPClient, div, r);
	Client->Send(msg, strlen(msg), 0);
	memset(msg, 0, sizeof(msg));
	Client->Receive(msg, BUFSIZ, 0);

	sscanf(msg, "%d", &codeftp);
	if (codeftp != 200)
	{
		replylogcode(codeftp);
		return 0;
	}
	if (Transfer->Create(port, SOCK_STREAM, NULL) == 0)
	{
		port++;
		return 0;
	}
	port++;
	printf("%s", msg);
	
	return 1;
}
int getServerPort(char *msg)
{
	int div, r;
	msg = strstr(msg, "(");
	for (int i = 0; i < 4; i++)
	{
		msg = strstr(msg + 1, ",");
	}
	sscanf(msg + 1, "%d", &div);
	msg = strstr(msg + 1, ",");
	sscanf(msg + 1, "%d", &r);
	return (div * 256 + r);
}
void logIn(CSocket *Client, char *info)
{
	char buf[BUFSIZ + 1];
	int codeftp;
	//USER
	if (stricmp(info, "user") == 0)
	{
		printf("Username: ");
		memset(buf, 0, sizeof(buf));
		scanf("%s", info);
		sprintf(buf, "USER %s\r\n", info);
	}
	else
	{
		sprintf(buf, "USER %s\r\n", info + 5);
	}

	Client->Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof(buf));
	Client->Receive(buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);

	if (codeftp != 331)
	{
		replylogcode(codeftp);
		return;
	}
	printf("%s", buf);
	//PASS
	printf("Password: ");
	memset(buf, 0, sizeof(buf));
	scanf("%s", info);

	sprintf(buf, "PASS %s\r\n", info);
	Client->Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof(buf));
	Client->Receive(buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 230)
	{
		replylogcode(codeftp);
		return;
	}
	printf("%s", buf);
}

void listDIR(CSocket *ClientSocket, char *dir, MODE mode)
{
	CSocket TransferSocket;
	char buf[BUFSIZ + 1];
	int codeftp, tmpres;
	if (mode == MODE::PASSIVE)
	{
		int portServer;
		sprintf(buf, "PASV\r\n");
		ClientSocket->Send(buf, strlen(buf), 0);
		memset(buf, 0, sizeof(buf));
		ClientSocket->Receive(buf, BUFSIZ, 0);
		printf("%s", buf);
		portServer = getServerPort(buf);


		if (TransferSocket.Create() == 0)
		{

			return ;
		}

		if (TransferSocket.Connect(_T("192.168.85.3"), portServer) == 0)
		{
			return ;
		}

	}
	else
	{
		if (sendPORT(ClientSocket, &TransferSocket) == 0)
		{
			return ;
		}
	}
	if (stricmp(dir, "dir") == 0)
	{
		sprintf(buf, "LIST\r\n");
	}
	else
	{
		sprintf(buf, "LIST %s\r\n", dir + 4);
	}
	ClientSocket->Send(buf, strlen(buf), 0);
	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 150)
	{
		if (codeftp == 550)
		{
			printf("No such file or directory\n");
		}
		replylogcode(codeftp);
		TransferSocket.Close();
		return ;
	}
	printf("%s", buf);


	// Ket noi den Port 20 cua Server de nhan,truyen du lieu
	if (mode == MODE::ACTIVE)
	{
		if (TransferSocket.Connect(_T("192.168.85.3"), 20) == 0)
		{
			return ;

		}
	}
	memset(buf, 0, sizeof(buf));
	while (tmpres = TransferSocket.Receive(buf, BUFSIZ, 0) > 0)
	{
		printf("%s", buf);
		memset(buf, 0, sizeof(buf));
	}
	TransferSocket.Close();
	ClientSocket->Receive(buf, BUFSIZ, 0);
	printf("%s", buf);

}

void nameListLS(CSocket *ClientSocket, char *ls, MODE mode)
{
	CSocket TransferSocket;
	char buf[BUFSIZ + 1];
	int codeftp, tmpres;
	if (mode == MODE::PASSIVE)
	{
		int portServer;
		sprintf(buf, "PASV\r\n");
		ClientSocket->Send(buf, strlen(buf), 0);
		memset(buf, 0, sizeof(buf));
		ClientSocket->Receive(buf, BUFSIZ, 0);
		printf("%s", buf);
		portServer = getServerPort(buf);

		if (TransferSocket.Create() == 0)
		{
			return ;
		}

		/*if (TransferSocket.Connect(_T("192.168.85.3"), portServer) != 0)
		{
			return 0;
		}*/
		if (TransferSocket.Connect(_T("192.168.85.3"), portServer) == 0)
		{
			return ;
		}
	}
	else
	{
		if (sendPORT(ClientSocket, &TransferSocket) == 0)
		{
			return ;
		}
	}
	if (stricmp(ls, "ls") == 0)
	{
		sprintf(buf, "NLST\r\n");
	}
	else
	{
		sprintf(buf, "NLST %s\r\n", ls + 3);
	}
	ClientSocket->Send(buf, strlen(buf), 0);
	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 150)
	{
		if (codeftp == 550)
		{
			printf("No such file or directory\n");
		}
		replylogcode(codeftp);
		TransferSocket.Close();
		return ;
	}
	printf("%s", buf);


	// Ket noi den Port 20 cua Server de nhan,truyen du lieu
	if (mode == MODE::ACTIVE)
	{
		if (TransferSocket.Connect(_T("192.168.85.3"), 20) == 0)
		{
			return ;

		}
	}
	memset(buf, 0, sizeof(buf));
	while (tmpres = TransferSocket.Receive(buf, BUFSIZ, 0) > 0)
	{
		printf("%s", buf);
		memset(buf, 0, sizeof(buf));
	}
	TransferSocket.Close();
	ClientSocket->Receive(buf, BUFSIZ, 0);
	printf("%s", buf);
}

int nameListWithLessMessageLS(CSocket *ClientSocket, char *ls)
{
	CSocket TransferSocket;
	char buf[BUFSIZ + 1];
	int codeftp, tmpres;

	int div = port / 256;
	int r = port % 256;
	sprintf(buf, "PORT %s,%d,%d\r\n", IPClient, div, r);
	ClientSocket->Send(buf, strlen(buf), 0);
	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 200)
	{
		replylogcode(codeftp);
		return 0;
	}
	if (TransferSocket.Create(port, SOCK_STREAM, NULL) == 0)
	{
		return 0;
	}
	port++;

	if (stricmp(ls, "ls") == 0)
	{
		sprintf(buf, "NLST\r\n");
	}
	else
	{
		sprintf(buf, "NLST %s\r\n", ls + 3);
	}
	ClientSocket->Send(buf, strlen(buf), 0);
	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 150)
	{
		TransferSocket.Close();
		printf("Directory not found\n");
		return 0;
	}

	// Ket noi den Port 20 cua Server de nhan,truyen du lieu
	if (TransferSocket.Connect(_T("192.168.85.3"), 20) == 0)
	{
		return 0;

	}

	memset(buf, 0, sizeof(buf));
	while (tmpres = TransferSocket.Receive(buf, BUFSIZ, 0) > 0)
	{
		memset(buf, 0, sizeof(buf));
	}
	TransferSocket.Close();
	ClientSocket->Receive(buf, BUFSIZ, 0);
	return 1;
}

void downloadFileGET(CSocket *ClientSocket, char *file, MODE mode)
{
	CSocket TransferSocket;
	char buf[BUFSIZ + 1];
	int codeftp, tmpres;
	char fileLocal[50];
	char fileRemote[50];
	char fileDir[100];

	if (mode == MODE::PASSIVE)
	{
		int portServer;
		sprintf(buf, "PASV\r\n");
		ClientSocket->Send(buf, strlen(buf), 0);
		memset(buf, 0, sizeof(buf));
		ClientSocket->Receive(buf, BUFSIZ, 0);
		printf("%s", buf);
		portServer = getServerPort(buf);


		if (TransferSocket.Create() == 0)
		{
			return;
		}

		if (TransferSocket.Connect(_T("192.168.85.3"), portServer) == 0)
		{
			return ;
		}

	}
	else
	{
		if (sendPORT(ClientSocket, &TransferSocket) == 0)
		{
			return;
		}
	}

	if (stricmp(file, "get") == 0)
	{
		printf("Remote File: ");
		scanf("%s", fileRemote);
		printf("Local File: ");
		scanf("%s", fileLocal);
	}
	else
	{
		strcpy(fileLocal, file + 4);
		strcpy(fileRemote, file + 4);
	}

	sprintf(buf, "RETR %s\r\n", fileRemote);
	ClientSocket->Send(buf, strlen(buf), 0);
	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);

	printf("%s", buf);
	sscanf(buf, "%d", &codeftp);
	if (codeftp != 150)
	{
		TransferSocket.Close();
		return;
	}

	memset(buf, 0, sizeof(buf));
	if (mode == MODE::ACTIVE)
	{
		if (TransferSocket.Connect(_T("192.168.85.3"), 20) == 0)
		{
			return ;
		}
	}

	sprintf(fileDir, "%s\\%s", lcd, fileLocal);
	FILE *fp = fopen(fileDir, "w");
	while (tmpres = TransferSocket.Receive(buf, BUFSIZ, 0) > 0)
	{
		fputs(buf, fp);
		memset(buf, 0, sizeof(buf));
	}
	TransferSocket.Close();
	ClientSocket->Receive(buf, BUFSIZ, 0);
	printf("%s", buf);

	fclose(fp);

}

void downloadFilesMGET(CSocket *ClientSocket, char *files, MODE mode)
{
	CSocket TransferSocket;
	char buf[BUFSIZ + 1];
	int codeftp, tmpres;
	char fileRemote[50];
	char fileDir[100];
	char *fileName;
	char tmp[50];
	char *tmpName;
	int flag[50] = { 0 };
	int i = 0;
	int sent = 0;

	if (stricmp(files, "mget") == 0)
	{

		printf("Remote File: ");
		fflush(stdin);
		scanf("%[^\n]", fileRemote);
	}
	else
	{
		strcpy(fileRemote, files + 4);
	}

	sprintf(buf, "TYPE A\r\n");
	ClientSocket->Send(buf, strlen(buf), 0);
	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);

	strcpy(tmp, fileRemote);
	tmpName = strtok(tmp, " ");
	while (tmpName != NULL)
	{
		char cmd[50] = "ls ";
		strcat(cmd, tmpName);
		if (nameListWithLessMessageLS(ClientSocket, cmd) == 1)
		{
			flag[i] = 1;
			sent = 1;
		}
		i++;
		tmpName = strtok(NULL, " ");
	}

	sprintf(buf, "TYPE A\r\n");
	ClientSocket->Send(buf, strlen(buf), 0);
	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);
	printf("%s", buf);

	i = 0;
	fileName = strtok(fileRemote, " ");
	while (fileName != NULL)
	{
		char tmpc[50];
		if (flag[i] == 1)
		{
			fflush(stdin);
			printf("mget %s?", fileName);
			gets(tmpc);
			if (strlen(tmpc) == 0) {
				if (mode == MODE::PASSIVE)
				{
					int portServer;
					sprintf(buf, "PASV\r\n");
					ClientSocket->Send(buf, strlen(buf), 0);
					memset(buf, 0, sizeof(buf));
					ClientSocket->Receive(buf, BUFSIZ, 0);
					printf("%s", buf);
					portServer = getServerPort(buf);

					if (TransferSocket.Create() == 0)
					{
						return;
					}

					if (TransferSocket.Connect(_T("192.168.85.3"), portServer) == 0)
					{
						return ;
					}
				}
				else
				{
					if (sendPORT(ClientSocket, &TransferSocket) == 0)
					{
						return;
					}
				}
				sprintf(buf, "RETR %s\r\n", fileName);
				ClientSocket->Send(buf, strlen(buf), 0);
				memset(buf, 0, sizeof(buf));
				ClientSocket->Receive(buf, BUFSIZ, 0);

				sscanf(buf, "%d", &codeftp);
				if (codeftp != 150)
				{
					replylogcode(codeftp);
					printf("%s", buf);
					TransferSocket.Close();
					return;
				}

				printf("%s", buf);

				if (mode == MODE::ACTIVE)
				{
					if (TransferSocket.Connect(_T("192.168.85.3"), 20) == 0)
					{
						return ;
					}
				}
				memset(buf, 0, sizeof(buf));

				sprintf(fileDir, "%s\\%s", lcd, fileName);
				FILE *fp = fopen(fileDir, "w");
				while (tmpres = TransferSocket.Receive(buf, BUFSIZ, 0) > 0)
				{
					fputs(buf, fp);
					memset(buf, 0, sizeof(buf));
				}
				TransferSocket.Close();
				ClientSocket->Receive(buf, BUFSIZ, 0);
				printf("%s", buf);

				fclose(fp);

			}
		}
		fileName = strtok(NULL, " ");
		i++;
	}
	if (sent == 0)
	{
		printf("Cannot find list of remote files\n");
	}
}

void UploadFilePUT(CSocket *ClientSocket, char *file, MODE mode)
{
	CSocket TransferSocket;
	char buf[BUFSIZ + 1];
	int codeftp, tmpres;
	char fileLocal[50];
	char fileRemote[50];
	char fileDir[100];

	if (stricmp(file, "put") == 0)
	{
		printf("Remote File: ");
		scanf("%s", fileRemote);
		printf("Local File: ");
		scanf("%s", fileLocal);
	}
	else
	{
		strcpy(fileLocal, file + 4);
		strcpy(fileRemote, file + 4);
	}

	sprintf(fileDir, "%s\\%s", lcd, fileLocal);
	//Kiem tra file co ton tai
	struct stat test;
	if (!(stat(fileDir, &test) == 0 && (test.st_mode & S_IFREG)))
	{
		printf("%s: File not found\n", fileLocal);
		return;
	}

	if (mode == MODE::PASSIVE)
	{
		int portServer;
		sprintf(buf, "PASV\r\n");
		ClientSocket->Send(buf, strlen(buf), 0);
		memset(buf, 0, sizeof(buf));
		ClientSocket->Receive(buf, BUFSIZ, 0);
		printf("%s", buf);
		portServer = getServerPort(buf);

		if (TransferSocket.Create() == 0)
		{
			return;
		}

		if (TransferSocket.Connect(_T("192.168.85.3"), portServer) == 0)
		{
			return ;
		}

	}
	else
	{
		if (sendPORT(ClientSocket, &TransferSocket) == 0)
		{
			return;
		}
	}

	sprintf(buf, "STOR %s\r\n", fileRemote);
	ClientSocket->Send(buf, strlen(buf), 0);
	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 150)
	{
		replylogcode(codeftp);
		TransferSocket.Close();
		return;
	}

	printf("%s", buf);

	// Ket noi den Server
	if (mode == MODE::ACTIVE)
	{
		if (TransferSocket.Connect(_T("192.168.85.3"), 20) == 0)
		{
			return ;
		}
	}
	memset(buf, 0, sizeof(buf));


	FILE *fp = fopen(fileDir, "rb");

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *str = (char*)malloc(size + 1);
	fread(str, size, 1, fp);
	TransferSocket.Send(str, size, 0);
	TransferSocket.Close();

	ClientSocket->Receive(buf, BUFSIZ, 0);
	printf("%s", buf);

	fclose(fp);

}

void UploadFilesMPUT(CSocket *ClientSocket, char *files, MODE mode)
{
	CSocket TransferSocket;
	char buf[BUFSIZ + 1];
	int codeftp, tmpres;
	char fileLocal[50];
	char fileDir[100];
	char *fileName;


	if (stricmp(files, "put") == 0)
	{
		fflush(stdin);
		printf("Local File: ");
		scanf("%[^\n]", fileLocal);
	}
	else
	{
		strcpy(fileLocal, files + 5);
	}
	fileName = strtok(fileLocal, " ");
	while (fileName != NULL)
	{
		sprintf(fileDir, "%s\\%s", lcd, fileName);
		struct stat test;
		if (stat(fileDir, &test) == 0 && (test.st_mode & S_IFREG))
		{
			char tmp[50];
			printf("mput %s?", fileName);
			fflush(stdin);
			gets(tmp);
			if (strlen(tmp) == 0) {
				if (mode == MODE::PASSIVE)
				{
					int portServer;
					sprintf(buf, "PASV\r\n");
					ClientSocket->Send(buf, strlen(buf), 0);
					memset(buf, 0, sizeof(buf));
					ClientSocket->Receive(buf, BUFSIZ, 0);
					printf("%s", buf);
					portServer = getServerPort(buf);


					//Tao socket cho server, dang ky port la 1234, giao thuc TCP
					if (TransferSocket.Create() == 0) //SOCK_STREAM or SOCK_DGRAM.
					{
						return;
					}

					if (TransferSocket.Connect(_T("192.168.85.3"), portServer) == 0)
					{
						return ;
					}

				}
				else
				{
					if (sendPORT(ClientSocket, &TransferSocket) == 0)
					{
						return;
					}
				}

				sprintf(buf, "STOR %s\r\n", fileName);
				ClientSocket->Send(buf, strlen(buf), 0);
				memset(buf, 0, sizeof(buf));
				ClientSocket->Receive(buf, BUFSIZ, 0);

				sscanf(buf, "%d", &codeftp);
				if (codeftp != 150)
				{
					replylogcode(codeftp);
					printf("%s", buf);
					TransferSocket.Close();
				}
				else
				{
					printf("%s", buf);

					// Ket noi den Server
					if (mode == MODE::ACTIVE)
					{
						if (TransferSocket.Connect(_T("192.168.85.3"), 20) == 0)
						{
							return ;
						}
					}
					memset(buf, 0, sizeof(buf));

					FILE *fp = fopen(fileDir, "rb");
					fseek(fp, 0, SEEK_END);
					long size = ftell(fp);
					fseek(fp, 0, SEEK_SET);
					char *str = (char*)malloc(size + 1);
					fread(str, size, 1, fp);
					TransferSocket.Send(str, size, 0);
					TransferSocket.Close();

					ClientSocket->Receive(buf, BUFSIZ, 0);
					printf("%s", buf);
					fclose(fp);
				}
			}
		}
		else
		{
			printf("%s: File not found\n", fileName);
		}
		fileName = strtok(NULL, " ");
	}
}

void localChangeDirectoryLCD(char *info)
{
	if (strcmp(info, "lcd") == 0)
	{
		printf("Local directory now: %s\n", lcd);
	}
	else
	{
		info = info + 4;
		struct stat test;
		if (stat(info, &test) == 0 && (test.st_mode & S_IFDIR))
		{
			lcd = info;
			printf("Local directory now: %s\n", lcd);
		}
		//if (PathFileExists(LPCWSTR(info)) == 0)
		//{
		//	printf("%s: Directory not found",info);
		//	return;
		//}
		else
		{
			printf("%s: No such file or directory\n", info);
			return;
		}
	}
}

void changeDirectoryCD(CSocket *ClientSocket, char *cd)
{
	char buf[BUFSIZ + 1];
	int codeftp;
	char remoteDir[50];

	if (stricmp(cd, "cd") == 0)
	{
		printf("Remote directory: ");
		scanf("%s", remoteDir);
	}
	else
	{
		strcpy(remoteDir, cd + 3);
	}
	sprintf(buf, "CWD %s\r\n", remoteDir);
	ClientSocket->Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 250)
	{
		printf("%s", buf);
		return;
	}
	printf("%s", buf);
}

void fileDELETE(CSocket *ClientSocket, char *file)
{
	char buf[BUFSIZ + 1];
	int codeftp;
	char fileName[50];
	if (stricmp(file, "delete") == 0)
	{
		printf("Remote file: ");
		memset(buf, 0, sizeof(buf));
		scanf("%s", fileName);
	}
	else
	{
		strcpy(fileName, file + 7);
	}
	sprintf(buf, "DELE %s\r\n", fileName);
	ClientSocket->Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 250)
	{
		printf("%s", buf);
		return;
	}
	printf("%s", buf);
}

void filesMDELETE(CSocket *ClientSocket, char *files)
{
	char buf[BUFSIZ + 1];
	int codeftp;
	char *fileName;
	char fileRemote[50];
	char tmp[50];
	char *tmpName;
	int flag[50] = { 0 };
	int i = 0;
	int sent = 0;

	if (stricmp(files, "mdelete") == 0)
	{
		printf("Remote File: ");
		fflush(stdin);
		scanf("%[^\n]", fileRemote);
	}
	else
	{
		strcpy(fileRemote, files + 8);
	}

	sprintf(buf, "TYPE A\r\n");
	ClientSocket->Send(buf, strlen(buf), 0);
	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);

	strcpy(tmp, fileRemote);
	tmpName = strtok(tmp, " ");
	//Kiem tra file co ton tai. Neu co bat flag[i] = 1
	while (tmpName != NULL)
	{
		char cmd[50] = "ls ";
		strcat(cmd, tmpName);
		if (nameListWithLessMessageLS(ClientSocket, cmd) == 1)
		{
			flag[i] = 1;
			sent = 1;
		}
		i++;
		tmpName = strtok(NULL, " ");
	}

	sprintf(buf, "TYPE A\r\n");
	ClientSocket->Send(buf, strlen(buf), 0);
	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);
	printf("%s", buf);

	i = 0;
	fileName = strtok(fileRemote, " ");
	while (fileName != NULL)
	{
		char tmp[50];
		if (flag[i] == 1)
		{
			printf("mdelete %s?", fileName);
			fflush(stdin);
			gets(tmp);
			if (strlen(tmp) == 0) {
				sprintf(buf, "DELE %s\r\n", fileName);
				ClientSocket->Send(buf, strlen(buf), 0);

				memset(buf, 0, sizeof(buf));
				ClientSocket->Receive(buf, BUFSIZ, 0);

				sscanf(buf, "%d", &codeftp);
				if (codeftp != 250)
				{
					replylogcode(codeftp);
					printf("%s", buf);
					return;
				}
				printf("%s", buf);
			}
		}
		fileName = strtok(NULL, " ");
		i++;
	}
	if (sent == 0)
	{
		printf("Cannot find list of remote files\n");
	}
}
void makeDirectoryMKDIR(CSocket *ClientSocket, char *folder)
{
	char buf[BUFSIZ + 1];
	int codeftp;
	char dirName[50];
	if (stricmp(folder, "mkdir") == 0)
	{
		printf("Directory name: ");
		memset(buf, 0, sizeof(buf));
		scanf("%s", dirName);
	}
	else
	{
		strcpy(dirName, folder + 6);
	}
	sprintf(buf, "XMKD %s\r\n", dirName);
	ClientSocket->Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 257)
	{
		replylogcode(codeftp);
		printf("%s", buf);
		return;
	}
	printf("%s", buf);
}

void removeDirectoryRMDIR(CSocket *ClientSocket, char *folder)
{
	char buf[BUFSIZ + 1];
	int codeftp;
	char dirName[50];
	if (stricmp(folder, "rmdir") == 0)
	{
		printf("Directory name: ");
		memset(buf, 0, sizeof(buf));
		scanf("%s", dirName);
	}
	else
	{
		strcpy(dirName, folder + 6);
	}
	sprintf(buf, "XRMD %s\r\n", dirName);
	ClientSocket->Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 250)
	{
		replylogcode(codeftp);
		printf("%s", buf);
		return;
	}
	printf("%s", buf);
}

void PWD(CSocket *ClientSocket)
{
	char buf[BUFSIZ + 1];
	int codeftp;

	sprintf(buf, "XPWD\r\n");
	ClientSocket->Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 257)
	{
		replylogcode(codeftp);
		printf("%s", buf);
		return;
	}
	printf("%s", buf);
}

int quitOrExit(CSocket *ClientSocket)
{
	char buf[BUFSIZ + 1];
	int codeftp;

	sprintf(buf, "QUIT\r\n");
	ClientSocket->Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof(buf));
	ClientSocket->Receive(buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 221)
	{
		printf("%s", buf);
		return 0;
	}
	printf("%s", buf);
	return 1;
}

void replylogcode(int code)
{
	switch (code) {
	case 200:
		printf("Command okay");
		break;
	case 500:
		printf("Syntax error, command unrecognized.");
		printf("This may include errors such as command line too long.");
		break;
	case 501:
		printf("Syntax error in parameters or arguments.");
		break;
	case 202:
		printf("Command not implemented, superfluous at this site.");
		break;
	case 502:
		printf("Command not implemented.");
		break;
	case 503:
		printf("Bad sequence of commands.");
		break;
	case 530:
		printf("Not logged in.");
		break;
	case 550:
		break;
	}
	printf("\n");
}
