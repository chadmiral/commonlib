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
      std::string tmp = str();
      std::cout << str();
     
      //count preceding tabs
      uint32_t num_tabs = 0;
      for (uint32_t i = 0; i < tmp.size(); i++)
      {
        if (tmp[i] == '\t') { num_tabs++; }
      }

      //write to log file
      _file_stream << "<p style=\"margin-left: " << 40 * num_tabs << "px\">";
      
      if (num_tabs == 0)
      {
        _file_stream << "<b>";
      }
      
      //chop off the \n & replace w/ <br>
      //_file_stream << tmp.substr(0, tmp.size() -1) << "<br>" << std::endl;

      _file_stream << tmp;

      if (num_tabs == 0)
      {
        _file_stream << "</b>";
      }

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