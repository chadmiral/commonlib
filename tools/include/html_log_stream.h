#pragma once

#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <unordered_map>
#include "vector.h"

namespace Tool
{
  class HtmlLogStream : public std::stringbuf
  {
  struct ofstream_pair
  {
    std::string     _channel_name;
    std::string     _fname;
    std::ofstream  *_ofs;
  };
  typedef std::unordered_map<uint32_t, ofstream_pair> FileMap;

  public:
    HtmlLogStream(std::string title = std::string("General")) :
      _mirror_stream(std::cout),
      std::stringbuf()
    {
      add_channel(title);
    }

    void add_channel(std::string channel_name)
    {
      ofstream_pair ofsp;
      ofsp._ofs = new std::ofstream;
      ofsp._fname = channel_name + ".html";
      ofsp._channel_name = channel_name;

      _curr_channel = Math::hash_value_from_string(channel_name.c_str());
      _file_streams.insert(std::make_pair(_curr_channel, ofsp));
    }

    void set_channel(std::string channel_name)
    {
      _curr_channel = Math::hash_value_from_string(channel_name.c_str());
    }

    std::string parse_string(std::string s)
    {
      std::string ret;
      bool need_closure = false;
      for (uint32_t i = 0; i < s.size(); i++)
      {
        switch (s[i])
        {
        case '\31':
          if (need_closure) { ret = ret + "</span>"; }
          ret = ret + "<span class=\"fatal_error\">";
          need_closure = true;
          break;
        case '\32':
          if (need_closure) { ret = ret + "</span>"; }
          ret = ret + "<span class=\"success\">";
          need_closure = true;
          break;
        case '\33':
          if (need_closure) { ret = ret + "</span>"; }
          ret = ret + "<span class=\"warning\">";
          need_closure = true;
          break;
        case '\34':
          if (need_closure) { ret = ret + "</span>"; }
          ret = ret + "<span class=\"blue\">";
          need_closure = true;
          break;
        default:
          ret = ret + s[i];
          break;
        }
      }
      if (need_closure)
      {
        ret = ret + "</span>";
      }
      return ret;
    }

    //called when a std::endl is received
    virtual int sync()
    {
      ofstream_pair ofsp = _file_streams[_curr_channel];

      std::string tmp = str();
      _mirror_stream << str();

      uint32_t num_tabs = 0;
      for (uint32_t i = 0; i < tmp.size(); i++)
      {
        if (tmp[i] == '\t' && num_tabs == i) { num_tabs++; }
      }

      //write to log file
      *ofsp._ofs << "<div ";
      *ofsp._ofs << "style = \"margin-left: " << 40 * num_tabs << "px\">";

      tmp = parse_string(tmp.substr(0, tmp.size() - 1)); //substring removes the endl
      *ofsp._ofs << tmp;

      *ofsp._ofs << "</div>";
      *ofsp._ofs << "<br>" << std::endl;

      str(""); //clear buffer
      return 0;
    }

    void open()
    {
      //TODO: load this from a file?
      static const char *css =
        "body {\n"
        "  font-family: Ubuntu;\n"
        "  font-size: 12px;\n"
        "  font-weight: 50;\n"
        "  background-color: #222222;\n"
        "  color: #ffffff;\n"
        "  width: 80%;\n"
        "  border: 1px solid #ffffff;\n"
        "  border-radius: 8px;\n"
        "  padding: 25px 25px 25px 25px;\n"
        "  margin: auto;\n"
        "  line-height: 8px;\n"
        "}\n"
        "\n"
        ".title {\n"
        "  font-family: Ubuntu;\n"
        "  font-size: 32px;\n"
        "  color: #ffffff;\n"
        "  text-align: right;\n"
        "}\n"
        ".date {\n"
        "  font-family:Ubuntu;\n"
        "  font-size: 14px;\n"
        "  color: #aaaaaa;\n"
        "  text-align: left;\n"
        "}\n"
        "\n"
        ".blue {\n"
        "  color:#4488ff;\n"
        "}\n"
        "\n"
        ".success {\n"
        "  color:#00ff00;\n"
        "}\n"
        "\n"
        ".fatal_error {\n"
        "  color:#ff0000;\n"
        "}\n"
        "\n"
        ".warning {\n"
        "  color:#ffff00;\n"
        "}\n";

      time_t t = time(0);   // get time now
      struct tm * now = localtime(&t);

      FileMap::iterator fs;
      for (fs = _file_streams.begin(); fs != _file_streams.end(); fs++)
      {
        fs->second._ofs->open(fs->second._fname);
        *(fs->second._ofs) << "<html>" << std::endl;
        *(fs->second._ofs) << "<head>" << std::endl;
        *(fs->second._ofs) << "<title>" << fs->second._channel_name << "</title>" << std::endl;
        *(fs->second._ofs) << "<link rel=\"stylesheet\" type=\"text/css\" href=\"http://fonts.googleapis.com/css?family=Ubuntu\" />" << std::endl;
        *(fs->second._ofs) << "<style>" << std::endl;
        *(fs->second._ofs) << css << std::endl;
        *(fs->second._ofs) << "</style>" << std::endl;
        *(fs->second._ofs) << "</head>" << std::endl;
        *(fs->second._ofs) << "<body>" << std::endl;
        *(fs->second._ofs) << "<div class=\"title\">" << fs->second._channel_name << "</div><br>" << std::endl;
        *(fs->second._ofs) << "<div class=\"date\">" << now->tm_mon + 1 << "/" << now->tm_mday << "/" << now->tm_year + 1900;
        *(fs->second._ofs) << " : " << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << "</div>" << std::endl;
        *(fs->second._ofs) << "<hr>" << std::endl;
        *(fs->second._ofs) << std::endl;
      }
    }

    void close()
    {
      FileMap::iterator i;
      for (i = _file_streams.begin(); i != _file_streams.end(); i++)
      {
        *(i->second._ofs) << "\t</body>" << std::endl;
        *(i->second._ofs) << "</html>" << std::endl;
        i->second._ofs->close();
        delete i->second._ofs;
      }
    }

  private:
    std::ostream  &_mirror_stream; //to (potentially) replace cout

    uint32_t      _curr_channel;
    FileMap       _file_streams;
  };
};
