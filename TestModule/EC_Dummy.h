// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TestSystem_EC_Dummy_h
#define incl_TestSystem_EC_Dummy_h

#include "ComponentInterface.h"
#include "Foundation.h"

namespace Test
{
    //! dummy componenent
    class EC_Dummy : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_Dummy);

    public:
        EC_Dummy &operator= (const EC_Dummy &other)
        {
            if (this != &other) // No self assignment
            {
                dummy_data1_ = other.dummy_data1_;
                dummy_data2_ = other.dummy_data2_;
            }

            return *this;
        }

        virtual ~EC_Dummy() {}
        
        int GetDummyData1() { return dummy_data1_; }
        void SetDummyData1(int data) { dummy_data1_ = data; }

        int GetDummyData2() { return dummy_data2_; }
        void SetDummyData2(int data) { dummy_data2_ = data; }
    private:
        //! default constructor, private since we implicitly use factory
        EC_Dummy() {}
        //! copy constructor, private since we implicitly use factory
        EC_Dummy(const EC_Dummy &rhs) : dummy_data1_(rhs.dummy_data1_), dummy_data2_(rhs.dummy_data2_) {}

        int dummy_data1_;
        int dummy_data2_;
    };
}

#endif
