#pragma once

#include <fstream>
#include <sstream>
#include <iostream>
#include "vector.h"

namespace Tool
{
  class HtmlLogStream : public std::stringbuf
  {
  public:
    HtmlLogStream(std::string title = std::string("Title")) :
      std::stringbuf(),
      _title(title),
      _indentation(2)
    {}

    void parse_line(std::string s)
    {

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
        //case '\n':
        // if (need_closure) { ret = ret + "</span>"; }
        //  ret = ret + "<br>";
        //  need_closure = false;
        //  break;
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
      std::string tmp = str();
      std::cout << str();

      uint32_t num_tabs = 0;
      for (uint32_t i = 0; i < tmp.size(); i++)
      {
        if (tmp[i] == '\t' && num_tabs == i) { num_tabs++; }
      }

      //write to log file
      _file_stream << "<div ";
      _file_stream << "style = \"margin-left: " << 40 * num_tabs << "px\">";

      tmp = parse_string(tmp.substr(0, tmp.size() - 1)); //substring removes the endl
      _file_stream << tmp;

       _file_stream << "</div>";
      _file_stream << "<br>" << std::endl;

      str(""); //clear buffer
      return 0;
    }

    void open(std::string fname)
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
        "h1 {\n"
        "  font-family: Ubuntu;\n"
        "  font-size: 32px;\n"
        "  color: #ffffff;\n"
        "  text-align: right;\n"
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

      _file_stream.open(fname.c_str());
      _file_stream << "<html>" << std::endl;
      _file_stream << "<head>" << std::endl;
      _file_stream << "<title>" << _title << "</title>" << std::endl;
      _file_stream << "<link rel=\"stylesheet\" type=\"text/css\" href=\"http://fonts.googleapis.com/css?family=Ubuntu\" />" << std::endl;
      //_file_stream << "<link rel=\"stylesheet\" type=\"text/css\" href=\"mundus_log.css\">" << std:: endl;
      _file_stream << "<style>" << std::endl;
      _file_stream << css << std::endl;
      _file_stream << "</style>" << std::endl;
      _file_stream << "</head>" << std::endl;
      _file_stream << "<body>" << std::endl;
      _file_stream << "<h1>" << _title << "</h1>" << std::endl;
      _file_stream << "<hr>" << std::endl;
      _file_stream << std::endl;
    }

    void close()
    {
      _file_stream << "\t</body>" << std::endl;
      _file_stream << "</html>" << std::endl;
      _file_stream.close();
    }

    void indent() { _indentation++; }
    void outdent() { _indentation--; }

  private:
    //std::ostream  _mirror_stream; //to replace cout
    std::ofstream _file_stream;
    std::string   _title;
    uint8_t      _indentation;
  };
};
