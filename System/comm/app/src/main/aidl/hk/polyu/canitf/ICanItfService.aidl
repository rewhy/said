// ICanItfService.aidl
package hk.polyu.canitf;

// Declare any non-default types here with import statements

interface ICanItfService {
    /**
     * Demonstrates some basic types that you can use as parameters
     * and return values in AIDL.
     */
//    void basicTypes(int anInt, long aLong, boolean aBoolean, float aFloat,
//            double aDouble, String aString);

    void setServerAdd(String add);
}
