/*
 This file contains all function defiitions related to the file browser functionality. 
 The code for this page is based off of the FSbrowser example for the ESP-32, ut is highly modified to fit the functionality of ESPLC. 
*/

#include <CORE/UICore.h>
#include <PLC/PLC_Main.h>
#include <SPIFFS.h>

//holds the current upload
File fsUploadFile;

//format bytes
String formatBytes(size_t bytes) 
{
    if (bytes < 1024) 
    {
        return String(bytes) + "B";
    } 
    else if (bytes < (1024 * 1024)) 
    {
        return String(bytes / 1024.0) + "KB";
    } 
    else if (bytes < (1024 * 1024 * 1024)) 
    {
        return String(bytes / 1024.0 / 1024.0) + "MB";
    } 
    else 
    {
        return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
    }
}

String getContentType(const String &filename) 
{
    if (Core.getWebServer().hasArg("download")) 
    {
        return PSTR("application/octet-stream");
    } 
    else if (filename.endsWith(".html") || filename.endsWith(".htm") ) 
    {
        return transmission_HTML;
    } 
    else if (filename.endsWith(".css")) 
    {
        return PSTR("text/css");
    } 
    else if (filename.endsWith(".js")) 
    {
        return PSTR("application/javascript");
    } 
    else if ( filename.endsWith(".png") ) 
    {
        return PSTR("image/png");
    } 
    else if (filename.endsWith(".gif")) 
    {
        return PSTR("image/gif");
    } 
    else if (filename.endsWith(".jpg")) 
    {
        return PSTR("image/jpeg");
    } 
    else if (filename.endsWith(".ico")) 
    {
        return PSTR("image/x-icon");
    } 
    else if (filename.endsWith(".xml"))
    {
        return PSTR("text/xml");
    } 
    else if (filename.endsWith(".pdf")) 
    {
        return PSTR("application/x-pdf");
    } 
    else if (filename.endsWith(".zip")) 
    {
        return PSTR("application/x-zip");
    } 
    else if (filename.endsWith(".gz")) 
    {
        return PSTR("application/x-gzip");
    }

    return transmission_Plain;
}

bool fileExists(const String &path)
{
  bool yes = false;
  //SPIFFS.exists(path);
  File file = SPIFFS.open(path, FILE_READ);
  if(!file.isDirectory())
  {
    yes = true;
  }
  file.close();
  return yes;
}

bool UICore::handleFileRead(const String &path) 
{
    if (fileExists(path)) 
    {
        File file = SPIFFS.open(path, FILE_READ);

        getWebServer().streamFile(file, getContentType(path));
        file.close();
        return true;
    }
    
    return false;
}

void UICore::handleFileUpload() 
{
    if (getWebServer().uri() != editDir) 
    {
        return;
    }
    HTTPUpload& upload = getWebServer().upload();
    if (upload.status == UPLOAD_FILE_START) 
    {
        String filename = upload.filename;
        if (!filename.startsWith("/")) 
        {
            filename = "/" + filename;
        }
        fsUploadFile = SPIFFS.open(filename, FILE_WRITE);
    } 
    else if (upload.status == UPLOAD_FILE_WRITE) 
    {
        if (fsUploadFile) 
        {
            fsUploadFile.write(upload.buf, upload.currentSize);
        }
    } 
    else if (upload.status == UPLOAD_FILE_END) 
    {
        if (fsUploadFile) 
        {
            fsUploadFile.close();
        }
    }
}

void UICore::handleFileDelete() 
{
    if (getWebServer().args() == 0) 
    {
        return Core.getWebServer().send(500, transmission_Plain, http_file_bad_args);
    }

    String path = getWebServer().arg(0);
    if (path == "/") 
    {
        return getWebServer().send(500, transmission_Plain, http_file_forbidden);
    }
    if (!fileExists(path)) 
    {
        return getWebServer().send(404, transmission_Plain, http_file_not_found);
    }
    SPIFFS.remove(path);
    getWebServer().send(200, transmission_Plain, "");
    path = String();
}

void UICore::handleFileCreate() 
{
    if (getWebServer().args() == 0) 
    {
        return getWebServer().send(500, transmission_Plain, http_file_bad_args);
    }
    String path = getWebServer().arg(0);
    if (path == "/") 
    {
        return getWebServer().send(500, transmission_Plain, http_file_forbidden);
    }
    if (fileExists(path)) 
    {
        return getWebServer().send(500, transmission_Plain, http_file_exists);
    }

    File file = SPIFFS.open(path, FILE_WRITE);
    if (file) 
    {
        file.close();
    } 
    else 
    {
        getWebServer().send(500, transmission_Plain, http_file_creation_failed);
        return;
    }

    getWebServer().send(200, transmission_Plain, "");
    path = String();
}

void UICore::handleFileList() 
{
    if (!getWebServer().hasArg("dir")) 
    {
        getWebServer().send(500, transmission_Plain, http_file_bad_args);
        return;
    }

    File root = SPIFFS.open(getWebServer().arg("dir"));

    String output = "[";
    if(root.isDirectory())
    {
        File file = root.openNextFile();
        while(file)
        {
            if (output != "[") 
            {
                output += ',';
            }
            output += "{\"type\":\"";
            output += (file.isDirectory()) ? "dir" : "file";
            output += "\",\"name\":\"";
            output += String(file.name()).substring(1);
            output += "\"}";
            file = root.openNextFile();
        }
    }
    output += "]";
    getWebServer().send(200, "text/json", output);
}