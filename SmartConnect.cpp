#include <Arduino.h>
#include "Base642.h"
#include <string.h>
#include <stdio.h>
#include <Ethernet.h>

#include "SmartConnect.h"

void SmartConnect::begin(int sp) {
	wConnectionAvaiable = false;
	wSerialPort = sp;
}

/** -- HELPER FUNCTIONS **/

/**
 * Method to create base64 key for authenticate on platform.
 * @param login [description]
 * @param pass  [description]
 */
void SmartConnect::encrypt(char login[], char pass[]) {

    char loginPass[60];
    strcpy(loginPass,login);
    strcat(loginPass,":");
    strcat(loginPass,pass);
    base64_encode(AuthID,loginPass,strlen(loginPass));

}

/**
 * Method to do a hand shake on _client
 */
void SmartConnect::doHandShake() {
	int maxAttempts = 10, attempts = 0;
	while(_client.available() == 0 && attempts < maxAttempts) {
        delay(100);
        attempts++;
    }
}

/**
 * Method for clear all data on memory.
 */
void SmartConnect::doFlushData() {
	Serial.flush();
    _client.stop();
    //_client.flush();
    //free(wBuffer);
}

/**
 * Method to parse a result based on a pattern
 * @param  outStr       [description]
 * @param  wData        [description]
 * @param  pattern      [description]
 * @param  patternSize  [description]
 * @param  closeBracket [description]
 * @return              [description]
 */
char* SmartConnect::parseBasedPattern(char outStr[], char wData[], char pattern[], int patternSize, char closeBracket[]) {

	int wI=0;
	int wX=0;
	int wMC=0;
	int wN=0;
	int rI=0;
	int xi;

	int wDataSize;
	int patternLen = strlen(pattern);

    outStr[0] = '\0';

	Serial.println();
	Serial.print("Pattern length: ");
	Serial.print(patternSize);
	Serial.println();

	Serial.println("Procurando padrao: ");
	for(xi=0; xi < patternSize; xi++) {
		Serial.print(pattern[xi]);
		if (!pattern[xi]) {
			break;
		}
		delay(10);
	}
	Serial.println();

	bool match = false;

	wDataSize = strlen(wData);

	for (wI=0; wI < wDataSize; wI++) {

		if (patternSize > 0) {

			if (!match) {

				wMC=0;
				for(wX=0; wX <= patternSize-1; wX++) {
					if (pattern[wX] == wData[wX+wI]) {
						wMC++;
					}
				}

				if (wMC == patternSize) {
					match = true;
				}

			}

			int dataSizeBuffer;

			if (match) {

                dataSizeBuffer = wDataSize;
                for(wN=wI+wMC; wN < dataSizeBuffer; wN++) {

                    if (wData[wN] == closeBracket[0]) {
                        wN=dataSizeBuffer+1;
                        wI=wDataSize;
                        outStr[rI] = '\0';
                    } else {
                        outStr[rI] = wData[wN];
                    }

                    rI++;

                }

			}

		}

	}

    Serial.println("Procurando padrao... Done");

    Serial.print("Resultado encontrado: ");
    Serial.print(outStr);
    Serial.print(" Tamanho: ");
    Serial.println(sizeof(outStr));
    delay(1);

    return outStr;

}

/**
 * [SmartConnect::connected description]
 * @return [description]
 */
bool SmartConnect::connected() {
	return wConnectionAvaiable;
}

/** -- SMART FUNCTIONS **/

/**
 * Function to connect on S.M.A.R.T
 * @param  hostname [description]
 * @param  login    [description]
 * @param  pass     [description]
 * @param  port     [description]
 * @return          [description]
 */
bool SmartConnect::connect(String hostname, char login[], char pass[], int port) {

	bool result = false;

    hostname.toCharArray(wHostname,21);
    //login.toCharArray(wLogin,30);
    //pass.toCharArray(wPass,20);
    wLogin = login;
    wPass = pass;
    wPort = port;

    encrypt(wLogin, wPass);

    while (!connected()) {
    	Serial.println("connectando no host...");
    	if (connectOnHostname()) {
    		Serial.println("criando envelope");
    		envelopeRequest("GET", "from", "", "", "", "", AuthID, "");
            result = readEnvelopeConnect();
    	}

    }

    return result;

}

String SmartConnect::send(char* rtype, char* app, char* schema, char* path, char* stringData) {

    if (connected()) {

        Serial.println("Dispositivo logado");

        Serial.println("Enviando dados...");

        if (connectOnHostname()) {

            Serial.println("enviando envelope ... ");

            Serial.println(rtype);
            Serial.println(app);
            Serial.println(schema);
            Serial.println(path);
            Serial.println(stringData);
            Serial.println(AuthID);
            Serial.println(wSessionId);

            envelopeRequest("POST", rtype, app, schema, path, stringData, AuthID, wSessionId);
            //doFlushData();

            delay(1);

            //doHandShake();

            Serial.println("lendo resposta ... ");
            readEnvelopeResponse(true, true);

            delay(1);

            Serial.println("resposta adicionada ao buffer...");
            Serial.println(wBuffer);

            //delay(1);
            doFlushData();

        }

    }

    return wBuffer;

}

/**
 * [SmartConnect::connectOnHostname description]
 * @return [description]
 */
bool SmartConnect::connectOnHostname() {

	 while (!_client.connected()) {

        Serial.println(F("Tentando conectar..."));
        Serial.println(wHostname);
        _client.connect(wHostname, 80);

    }

}

/**
 * [SmartConnect::envelopeRequest description]
 * @param  rtype      [description]
 * @param  rpath      [description]
 * @param  app        [description]
 * @param  schema     [description]
 * @param  path       [description]
 * @param  stringData [description]
 * @param  AuthID     [description]
 * @param  PHPSESSID  [description]
 * @return            [description]
 */
void SmartConnect::envelopeRequest(char* rtype,char* rpath,char* app,char* schema,char* path,char* stringData,char* AuthID,char* PHPSESSID) {

    Serial.println(AuthID);
    Serial.println(PHPSESSID);
    Serial.println(wSessionId);

    Serial.println(F("Montando envelope..."));

    _client.print(rtype);
    _client.print(F(" /api/fp/"));
    _client.print(rpath);

    Serial.print(rtype);
    Serial.print(F(" /api/fp/"));
    Serial.print(rpath);

    if (app != "") {

        _client.print(F("/"));
        _client.print(app);

        Serial.print(F("/"));
        Serial.print(app);

    }

    if (schema != "") {

        _client.print(F("/"));
        _client.print(schema);

        Serial.print(F("/"));
        Serial.print(schema);
    }

    if (path != "") {

        _client.print(F("/"));
        _client.print(path);

        Serial.print(F("/"));
        Serial.print(path);

    }

    if (rtype == "GET") {

        _client.println(F(" HTTP/1.0"));

        //_client.print(F("Host: "));
        //_client.println(F("www.smartapps.com.br"));
        //_client.println(F("User-Agent: arduino-ethernet"));

        Serial.println(F(" HTTP/1.0"));

        //Serial.print(F("Host: "));
        //Serial.println(F("www.smartapps.com.br"));
        //Serial.println(F("User-Agent: arduino-ethernet"));

    } else {

        _client.println(F(" HTTP/1.1"));

        _client.print(F("Host: "));
        _client.println(F("www.smartapps.com.br"));
        _client.println(F("User-Agent: arduino-ethernet"));

        Serial.println(F(" HTTP/1.1"));

        Serial.print(F("Host: "));
        Serial.println(F("www.smartapps.com.br"));
        Serial.println(F("User-Agent: arduino-ethernet"));

    }

    if (AuthID[0] != '\0') {

        _client.print(F("Authorization: Basic "));
        _client.println(AuthID);

        Serial.print(F("Authorization: Basic "));
        Serial.println(AuthID);

    }

    if (PHPSESSID[0] != '\0') {

        _client.print(F("Cookie: PHPSESSID="));
        _client.println(PHPSESSID);

        Serial.print(F("Cookie: PHPSESSID="));
        Serial.println(PHPSESSID);

    }

    _client.println(F("Connection: close"));
    Serial.println(F("Connection: close"));

    if (rtype == "POST") {

        _client.println(F("Content-Type: application/x-www-form-urlencoded"));
        _client.print(F("Content-Length: "));
        _client.println(30);
        _client.println();
        _client.println(stringData);

        Serial.println(F("Content-Type: application/x-www-form-urlencoded"));
        Serial.print(F("Content-Length: "));
        Serial.println(30);
        Serial.println();
        Serial.println(stringData);

    }

    _client.println();
    Serial.println();

    delay(100);

    Serial.println("Envelope enviado.");

}

void SmartConnect::readEnvelopeResponse(bool d, bool excludeHeaders) {

    int maxAttempts = 9999, attempts = 0, timeout = 0;

    while(_client.available() == 0 && attempts < maxAttempts){
        delay(100);
        attempts++;
    }

    if (d) { Serial.println(attempts); }

    if (attempts == maxAttempts) {
        if (d) { Serial.println("Timeout _client when read response."); }
        timeout=1;
    }

    if (timeout == 0) {

        if (excludeHeaders) {

            char character;
            int posBuffer=0;
            int posResponseContent=0;
            int charBuffer=0;
            bool first=false;
            char lastChar[3];
            int lastCharInt=0;

            Serial.println(_client.available());

            memset(wBuffer,'\0',1280);

            while(_client.available() > 0 && (character = _client.read())) {

                if (d) {
                    Serial.println(character);
                }

                if (
                    character == '\n' &&
                    lastChar[0] == '\r' &&
                    lastChar[1] == '\n' &&
                    lastChar[2] == '\r'
                ) {
                    posResponseContent = 1;
                }

                if (posResponseContent == 1 && first && character != '\n') {
                    wBuffer[charBuffer] = character;
                    charBuffer++;
                } else if (posResponseContent == 1) {
                    first=true;
                }

                posBuffer++;

                lastChar[lastCharInt]=character;

                if (lastCharInt == 2) {
                    lastCharInt=0;
                } else {
                    lastCharInt++;
                }


            }

            if (d) {
                Serial.println("BUFFER EH:");
                Serial.println(wBuffer);
            }

        } else {

            char character;
            int posBuffer=0;
            int posResponseContent=0;
            int charBuffer=0;

            memset(wBuffer,'\0',1280);

            while(_client.available() > 0 && (character = _client.read())) {

                if (d) { Serial.println(character); }

                if (character != '\n') {
                    wBuffer[charBuffer] = character;
                } else {
                    wBuffer[charBuffer] = '\n';
                }

                charBuffer++;

                posBuffer++;

            }

            if (d) {
                Serial.println("BUFFER BASIC:");
                Serial.println(wBuffer);
            }

        }

    }

}

bool SmartConnect::checkIfResponseIs200() {

	int i;
	i=0;

    if(
    	(wBuffer[i] == 'H') &&
        (wBuffer[i+1] == 'T') &&
        (wBuffer[i+2] == 'T') &&
        (wBuffer[i+3] == 'P') &&
        (wBuffer[i+4] == '/') &&
        (wBuffer[i+5] == '1') &&
        (wBuffer[i+6] == '.') &&
        (wBuffer[i+7] == '1') &&
        (wBuffer[i+8] == ' ')
      ){

        //verifica se a resposta eh 200
        if(
        	(wBuffer[i+9] == '2') &&
        	(wBuffer[i+10] == '0') &&
        	(wBuffer[i+11] == '0')
        ){
            return true;
        } else {
        	return false;
        }

    }

    return false;

}
/*
bool SmartConnect::checkIfStatusIsSuccess() {

	//procura se conexao foi "sucessfull"
	bool result;
	int i;

    while(_client.available()) {

        readLine();

        //#ifdef NEW
        result = -1;
        //-10 do Successfull
        for(i=0;i<posBuffer-10;i++){
            //procura o header
            if( (readBuffer[i] == 'S') &&
                (readBuffer[i+1] == 'u') &&
                (readBuffer[i+2] == 'c') &&
                (readBuffer[i+3] == 'c') &&
                (readBuffer[i+4] == 'e') &&
                (readBuffer[i+5] == 's') &&
                (readBuffer[i+6] == 's') &&
                (readBuffer[i+7] == 'f') &&
                (readBuffer[i+8] == 'u') &&
                (readBuffer[i+9] == 'l') &&
                (readBuffer[i+10] == 'l')
              ){
                //encontrou, sai do for
                result = true;
                break;
            }
        }
        //#endif
        /*
        #ifndef NEW
        result = buff.indexOf("Successfull") != -1;
        #endif

        if(result){
            wConnectionAvaiable = 1;
            wSessionId = PHPSESSID;
            return true;
            break;
        } else {
            wConnectionAvaiable = 0;
        }

    }

}*/

/**
 * Method to get and set sessionID
 */
void SmartConnect::getAndSetSessionId() {

    Serial.println(F("Procurando PHPSESSID"));
    delay(10);

    // process sessid
    char wPattern[6] = {'"','i','d','"',':','"'}; // "id":"
    char wCloseBracket[1] = {'"'}; // "

    parseBasedPattern(wSessionId, wBuffer, wPattern, 6, wCloseBracket);

    if (wSessionId) {

        Serial.print(F("PHPSESSID ID: "));
        Serial.println(wSessionId);

        // process sessid name
        char wPattern[8] = {'"','n','a','m', 'e','"',':','"'}; // "name":"
        char wCloseBracket[1] = {'"'}; // "

        parseBasedPattern(wSessionName, wBuffer, wPattern, 8, wCloseBracket);

        Serial.print(F("PHPSESSID NAME: "));
        Serial.println(wSessionName);

    }

}

/**
 * Method to read envelope called on 'connect' method.
 * @return [description]
 */
bool SmartConnect::readEnvelopeConnect() {

    Serial.print(F("hostnamec: "));
    Serial.println(wHostname);

    int time = millis();

    doHandShake();

    //procura se a resposta HTTP eh 200
    readEnvelopeResponse(true, false);
    delay(1);
    getAndSetSessionId();
    delay(1);

    if (wSessionId[0] != '\0') {
        Serial.println("Autenticado");
        wConnectionAvaiable = true;
    } else {
        wConnectionAvaiable = false;
    }

    doFlushData();

    return wConnectionAvaiable;

}