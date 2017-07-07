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

    //called when a std::endl is received
    virtual int sync()
    {
      std::cout << str();

      //write to log file
      for (uint8_t i = 0; i < _indentation; i++)
      {
        _file_stream << "<p style=\"margin-left: 40px\">";
      }
      
      //chop off the \n & replace w/ <br>
      std::string tmp = str();
      _file_stream << tmp.substr(0, tmp.size() -1) << "<br>" << std::endl;

      str(""); //clear buffer
      return 0;
    }

    void open(std::string fname)
    {
      _file_stream.open(fname.c_str());
      _file_stream << "<html>" << std::endl;
      
      /*
      _file_stream << "<head>" << std::endl;
      _file_stream << "\t<style type = \"text/css\">" << std::endl;
      _file_stream << "\t<!--" << std::endl;
      _file_stream << "\t\t.tab{ margin - left: 40px; }" << std::endl;
      _file_stream << "\t-->" << std::endl;
      _file_stream << "\t< / style>" << std::endl;
      _file_stream << "</head>" << std::endl;
      */
      
      _file_stream << "\t<body>" << std::endl;
      _file_stream << "\t<h1>" << _title << "</h1>" << std::endl;
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