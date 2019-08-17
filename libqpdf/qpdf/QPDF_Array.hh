#ifndef QPDF_ARRAY_HH
#define QPDF_ARRAY_HH

#include <qpdf/QPDFObject.hh>

#include <vector>
#include <qpdf/QPDFObjectHandle.hh>

class QPDF_Array: public QPDFObject
{
  public:
    QPDF_Array(std::vector<QPDFObjectHandle> const& items);
    virtual ~QPDF_Array();
    virtual std::string unparse();
    virtual JSON getJSON();
    virtual QPDFObject::object_type_e getTypeCode() const;
    virtual char const* getTypeName() const;
    virtual void setDescription(QPDF*, std::string const&);

    int getNItems() const;
    QPDFObjectHandle getItem(int n) const;
    void getAsVector(std::vector<QPDFObjectHandle>&) const;

    void setItem(int, QPDFObjectHandle const&);
    void setFromVector(std::vector<QPDFObjectHandle> const& items);
    void insertItem(int at, QPDFObjectHandle const& item);
    void appendItem(QPDFObjectHandle const& item);
    void eraseItem(int at);

  protected:
    virtual void releaseResolved();

  private:
    std::vector<QPDFObjectHandle> items;
};

#endif // QPDF_ARRAY_HH
