#include "help.h"
#include <SDL.h>
#include <SDL_filesystem.h>
#include <stdio.h>
#include <io.h>

const char* pendulum_help_hta = {
"<!-- save this as pendulum_help.hta -->\n"
"<HTML>\n"
"<HEAD>\n"
"    <TITLE>�������: ���������� ��������� ���������� ��������</TITLE>\n"
"    <HTA:APPLICATION \\r\\n"
"        ID=\"PendulumHelp\"\\r\\n"
"        APPLICATIONNAME=\"PendulumSimulatorHelp\"\\r\\n"
"        BORDER=\"thin\"\\r\\n"
"        BORDERSTYLE=\"complex\"\\r\\n"
"        CAPTION=\"yes\"\\r\\n"
"        SHOWINTASKBAR=\"yes\"\\r\\n"
"        SINGLEINSTANCE=\"yes\"\\r\\n"
"        SYSMENU=\"yes\"\\r\\n"
"        SCROLL=\"yes\"\\r\\n"
"        ICON=\"\"\\r\\n"
"        WINDOWSTATE=\"normal\"\\r\\n"
"        MAXIMIZEBUTTON=\"no\"\\r\\n"
"        MINIMIZEBUTTON=\"no\"\\r\\n"
"        WIDTH=\"600\"\\r\\n"
"        HEIGHT=\"500\"\\r\\n"
"    />\n"
"    <STYLE>\n"
"        body {\n"
"            font-family: 'Segoe UI', Tahoma, sans-serif;\n"
"            margin: 20px;\n"
"            background-color: #f9f9f9;\n"
"            color: #333;\n"
"            overflow: auto;\n"
"        }\n"
"        h1 {\n"
"            font-size: 1.8em;\n"
"            margin-bottom: 10px;\n"
"            border-bottom: 2px solid #000000;\n"
"            padding-bottom: 5px;\n"
"            color: #000000;\n"
"        }\n"
"        h2 {\n"
"            font-size: 1.2em;\n"
"            margin-top: 20px;\n"
"            color: #333;\n"
"        }\n"
"        p, li {\n"
"            font-size: 1em;\n"
"            line-height: 1.4em;\n"
"        }\n"
"        ul {\n"
"            padding-left: 20px;\n"
"        }\n"
"        kbd {\n"
"            background-color: #eaeaea;\n"
"            border: 1px solid #ccc;\n"
"            border-radius: 3px;\n"
"            box-shadow: inset 0 -1px 0 #bbb;\n"
"            padding: 2px 4px;\n"
"            font-size: 0.9em;\n"
"            font-family: Consolas, 'Courier New', monospace;\n"
"        }\n"
"        .footer {\n"
"            margin-top: 30px;\n"
"            font-size: 0.9em;\n"
"            color: #666;\n"
"            text-align: right;\n"
"        }\n"
"    </STYLE>\n"
"    <SCRIPT LANGUAGE=\"javascript\">\n"
"        window.resizeTo(600, 500);\n"
"        var x = (screen.availWidth - 600) / 2;\n"
"        var y = (screen.availHeight - 500) / 2;\n"
"        window.moveTo(x, y);\n"
"    </SCRIPT>\n"
"    <SCRIPT LANGUAGE=\"VBScript\">\n"
"    Sub window_onkeydown\n"
"        Select Case window.event.keyCode\n"
"            Case 27\n"
"                window.close\n"
"            Case 82\n"
"                MsgBox \"�������� ��������: ������� ������� R � �������� ���������\", vbInformation, \"��������\"\n"
"            Case Else\n"
"        End Select\n"
"    End Sub\n"
"    </SCRIPT>\n"
"</HEAD>\n"
"<BODY>\n"
"    <h1>������� �� ��������� \"���������� ��������� ���������� ��������\"</h1>\n"
"    <h2>1. ����� �������</h2>\n"
"    <p>��� ����������� ������� ����������� ���� ������� ������� <kbd>F1</kbd>.</p>\n"
"    <h2>2. ���������� ���������</h2>\n"
"    <ul>\n"
"        <li><strong>��������� ���� ����������:</strong> ��������� �������� �������� �������� ���� � ����������� �� ��������� ����.</li>\n"
"        <li><strong>������������� ��������� ������:</strong> ��������� ���� �������� ���� � ����������� ����� ��� ������� (������������ �����������).</li>\n"
"        <li><strong>�������� ��������:</strong> ������� ������� <kbd>R</kbd> ��� ���������� ��������.</li>\n"
"    </ul>\n"
"    <h2>3. ���������� ����������</h2>\n"
"    <ul>\n"
"        <li><strong>������ ����������� �������������:</strong> ����� ��������� ���������� ���� ���������� ������� ������ \"�����\" ��� ������� <kbd>Enter</kbd>.</li>\n"
"        <li><strong>������������ ���������:</strong> ������� ������ \"����\" ��� ������� <kbd>Escape</kbd>.</li>\n"
"        <li><strong>����� ����������:</strong> ������� ������ \"�����\" ��� ������ <kbd>Escape</kbd>.</li>\n"
"    </ul>\n"
"    <div class=\"footer\">\n"
"        ��� ������ �� ��������� ����������� Alt+F4.\n"
"    </div>\n"
"</BODY>\n"
"</HTML>\n"
};

bool mem_to_file(const char* ppath, 
  const char* pdata, size_t size)
{
  FILE* fp;
#if _MSC_VER
  fopen_s(&fp, ppath, "w");
#define access _access
#else
  fp = fopen(ppath, "w");
#endif
  if (!fp)
    return false;

  fputs(pendulum_help_hta, fp);
  fclose(fp);
  return true;
}

bool show_help()
{
  char fullpath[512], fullpath2[512];
  const char* ppath = SDL_GetPrefPath("NSTU_NETI", "pendulum_sim");
  if (ppath) {
    SDL_snprintf(fullpath, sizeof(fullpath), "%s%s", ppath, "help.hta");
    /* create file if no exists */
    if (access(fullpath, 0) != 0) {
      if (!mem_to_file(fullpath, pendulum_help_hta, SDL_strlen(pendulum_help_hta))) {
        SDL_ShowSimpleMessageBox(0, "Help error", "Save file error", nullptr);
        return false;
      }
    }

    SDL_snprintf(fullpath2, sizeof(fullpath2), "file:///%s", fullpath);
    printf("opening URL: \"%s\"", fullpath2);
    if (SDL_OpenURL(fullpath2) < 0) {
      SDL_snprintf(fullpath, sizeof(fullpath), 
        "SDL_OpenURL() failed!\nSDL_GetError(): %s\nURL: \"%s\"", 
        SDL_GetError(),
        fullpath2);
      SDL_ShowSimpleMessageBox(0, "Help error", fullpath, nullptr);
      return false;
    }
    return true;
  }
  return false;
}

#ifdef access
#undef access
#endif
