/*
 *
 *      goauth.c - library for wmHGmail to handle Google OAuth
 * 
 *	Author: Haydn Haines - haydnhdev@gmail.com
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2, or (at your option)
 *      any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program (see the file COPYING); if not, write to the
 *      Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 *      Boston, MA  02111-1307, USA
 *
 *
 *      Changes:
 *
 *      Version 1.00  - released 2017-11-09
 *
 */


// Includes  
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <curl/curl.h>
#include <json-c/json.h>


// Global Variables
extern int	uCount, aState;
extern char	fileConf[PATH_MAX];
char            atoken[150] = "", rtoken[150] = "", gcode[48] = "";


pthread_t  tid[2], ttid[2];

struct MemoryStruct {
  char *memory;
  size_t size;
};


// Check the config file exists and the permissions are correct
void checkConf(char *pathConf) {
  struct stat st;

  if (stat(pathConf, &st) != 0 || ! S_ISDIR(st.st_mode)) {
    mkdir(pathConf, 0700);
  }
  int fd = open(fileConf, O_RDWR | O_CREAT, 0600);
  close(fd);
}


// Read access and refresh tokens from the json config file
int readConf() {
  char *aTokJ = NULL;
  FILE *fp;
  fp = fopen(fileConf, "r");
  char *buffer = NULL;
  size_t len;
  size_t bytes_read = getdelim(&buffer, &len, '\0', fp);
  fclose(fp);

  struct json_object *jobj_in = NULL, *jobj_resR = NULL, *jobj_resA = NULL;

  if (bytes_read != -1) {
    jobj_in = json_tokener_parse(buffer);
    json_object_object_get_ex(jobj_in, "refresh_token", &jobj_resR);
    aTokJ = (char *) json_object_get_string(jobj_resR);
    if (aTokJ)
      strcpy(rtoken, aTokJ);

    json_object_object_get_ex(jobj_in, "access_token", &jobj_resA);
    aTokJ = (char *) json_object_get_string(jobj_resA);
    if (aTokJ)
      strcpy(atoken, aTokJ);
  }

  free(buffer);
  json_object_put(jobj_in);

  if (!aTokJ)
    return 1;
  return 0;
}


// Write the json string provided as an argument to the config file
void writeConf(const char *jsonStr) {
    FILE *fp;
    fp = fopen(fileConf, "w");
    if (fp != NULL)
    {
        fputs(jsonStr, fp);
        fclose(fp);
    }
}


// Convert a google json response to an unread count integer
void json2unread(char *unreadJson) {
  struct json_object *jobj_in = NULL, *jobj_out = NULL;
  jobj_in = json_tokener_parse(unreadJson);
  json_object_object_get_ex(jobj_in, "messagesUnread", &jobj_out);
  if (jobj_out) {
    uCount = json_object_get_int(jobj_out);
  }
  json_object_put(jobj_in);
}


// Convert google json response to an access token
void json2aToken(char *atokenJson) {
  struct json_object *jobj_in = NULL, *jobj_out = NULL;
  jobj_in = json_tokener_parse(atokenJson);
  json_object_object_get_ex(jobj_in, "access_token", &jobj_out);

  if (jobj_out) {
    json_object_object_get_ex(jobj_in, "access_token", &jobj_out);
    sprintf(atoken, "%s", json_object_get_string(jobj_out));
    json_object_object_add(jobj_in, "refresh_token", json_object_new_string(rtoken));
    writeConf(json_object_to_json_string_ext(jobj_in, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
  }
  json_object_put(jobj_in);
}


// Curl memory call back function
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}


// Curl write to file function
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}


// Function to perform http requests using curl, flowStage arguments:
//    1 - Request google access code.
//    2 - Exchange code for access & refresh tokens.
//    3 - Use the current access token to retrieve the unread count.
//    4 - Use the refresh token for a new access token.
long doCurl(int flowStage) {
  // Exit if we don't have a refresh token and are trying to refresh
  if (rtoken == 0 && !rtoken[0] && flowStage == 4) 
    return 401;

  char *cid = "531199373338-85eku0ccll075efa15gac3lho4qv4p9j.apps.googleusercontent.com";
  char *csec = "bqP-Ads47RN3W75Fb0LcgJZ7";
  char *ruri = "urn:ietf:wg:oauth:2.0:oob";
  char *scope = "https://www.googleapis.com/auth/gmail.readonly";
  char *url;
  char aurl[512] = "";
  char popts[512] = "";
  FILE *devnull = fopen("/dev/null", "w");
  int fd;
  CURL *curl;
  CURLcode res = 0;
  long httpres = 0;
  FILE *pagefile;
  struct MemoryStruct chunk;

  chunk.memory = malloc(1);
  chunk.size = 0;

  curl = curl_easy_init();

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, devnull);

    // Get access code
    if (flowStage == 1) {
      strcpy(aurl, "https://accounts.google.com/o/oauth2/v2/auth");
      sprintf(popts, "response_type=code&client_id=%s&redirect_uri=%s&scope=%s", cid, ruri, scope);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, popts);

    // Get an access & refresh token
    } else if (flowStage == 2) {
      strcpy(aurl, "https://www.googleapis.com/oauth2/v4/token");
      sprintf(popts, "code=%s&client_id=%s&client_secret=%s&redirect_uri=%s&grant_type=authorization_code", gcode, cid, csec, ruri);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, popts);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
      pagefile = fopen(fileConf, "wb");

      if(pagefile) {
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, pagefile);

      }

    // Get unread count
    } else if (flowStage == 3) {
      sprintf(aurl, "https://www.googleapis.com/gmail/v1/users/me/labels/UNREAD?access_token=%s", atoken);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
      curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    // Refresh access token
    } else if (flowStage == 4) {
      strcpy(aurl, "https://www.googleapis.com/oauth2/v4/token");
      sprintf(popts, "client_id=%s&client_secret=%s&refresh_token=%s&grant_type=refresh_token", cid, csec, rtoken);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, popts);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    }

    // Perform the request
    curl_easy_setopt(curl, CURLOPT_URL, aurl);
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      aState = 1;
      
    } else {
      curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpres);
      if (flowStage == 1) {
        int pid = fork();
        if (pid == 0) {
        // Child process
          fd = open("/dev/null",O_WRONLY | O_CREAT, 0666);
          dup2(fd, 1);
          dup2(fd, 2);
          execl("/bin/xdg-open", "/bin/xdg-open", url, (char *) 0);
          close(fd);
          exit(127);

        } else {
          signal(SIGCHLD, SIG_IGN);
          fprintf(stdout, "\nPlease check you browser for a Google approval form. This app will try to obtain a code automatically, if that fails please copy & paste the code and enter it below.\n\nCode: ");

        }
      } else if (flowStage == 2) {
        fclose(pagefile);

      } else if (flowStage == 3) {
        json2unread(chunk.memory);

      } else if (flowStage == 4) {
        json2aToken(chunk.memory);

      }
    }
    fclose(devnull);
    curl_easy_cleanup(curl);
    free(chunk.memory);

  }
  return httpres;
}


// Function to read the google code from the browser title or user entry
void *getCode(void *arg) {
  FILE *fp;
  char iBuf[48] = "";

  pthread_t id = pthread_self();

  if (pthread_equal(id, tid[0])) {
    while (!strcmp(gcode, "")) {
      fp = popen("wmctrl -l |egrep -o 'Success code=\\S*' |sed 's/Success code=//' 2>/dev/null", "r");
      if (fp) {
        fgets(gcode, 100, fp);
        strtok(gcode, "\n");
        usleep(5000L);
        pclose(fp);
      }
    }

  } else {
    while (!strcmp(gcode, "")) {
      fgets(iBuf, 47, stdin);
      if (strlen(iBuf) == 46)
        strcpy(gcode, iBuf);
    }
  }
  return NULL;
}


// Entry point for google authentication
void gAuth() {
  int err, tCount = 0;

  strcpy(gcode, "");
  doCurl(1);

  while (tCount < 2) {
    err = pthread_create(&(tid[tCount]), NULL, &getCode, NULL);
    if (err != 0 )
      fprintf(stderr, "Couldn't create thread: %s\n", strerror(err));
    tCount++;
  }

  while (!strcmp(gcode, "")) {
    usleep(50000L);
  }
  doCurl(2);
}

