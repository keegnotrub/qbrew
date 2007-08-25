/***************************************************************************
  style.h
  -------------------
  AHA-like style class
  -------------------
  Copyright 1999-2007, David Johnson
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

#ifndef STYLE_H
#define STYLE_H

#include <QList>
#include <QMap>
#include <QString>

class Style {
public:
    // default constructor
    Style();
    // constructor
    Style(const QString name, const double &oglow, const double &oghi,
          const double &fglow, const double &fghi, const int &ibulow,
          const int &ibuhi, const int &srmlow, const int &srmhi);
    // copy constructor
    Style(const Style &s);
    // operators
    Style operator=(const Style &s);
    bool operator==(const Style &s) const;
    bool operator<(const Style &s) const;
    // destructor
    ~Style();

    // return name of style
    const QString &name() const;
    void setName(const QString &name);
    // return high end of OG
    double OGHi() const;
    // return low end of OG
    double OGLow() const;
    // return high end of FG
    double FGHi() const;
    // return low end of FG
    double FGLow() const;
    // return high end of IBU
    int IBUHi() const;
    // return low end of IBU
    int IBULow() const;
    // return high end of SRM
    int SRMHi() const;
    // return low end of SRM
    int SRMLow() const;
    
private:
    friend class StyleModel;

    QString name_;
    double oglow_;
    double oghi_;
    double fglow_;
    double fghi_;
    int ibulow_;
    int ibuhi_;
    int srmlow_;
    int srmhi_;
};

typedef QList<Style> StyleList;
typedef QMap<QString,Style> StyleMap;

//////////////////////////////////////////////////////////////////////////////
// Inlined Methods

inline bool Style::operator<(const Style &s) const { return (name_<s.name_); }

inline const QString &Style::name() const { return name_; }

inline void Style::setName(const QString &name) { name_ = name; }

inline double Style::OGHi() const { return oghi_; }

inline double Style::OGLow() const { return oglow_; }

inline double Style::FGHi() const { return fghi_; }

inline double Style::FGLow() const { return fglow_; }

inline int Style::IBUHi() const { return ibuhi_; }

inline int Style::IBULow() const { return ibulow_; }

inline int Style::SRMHi() const { return srmhi_; }

inline int Style::SRMLow() const { return srmlow_; }

#endif // STYLE_H
