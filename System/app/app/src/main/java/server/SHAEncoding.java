package server;

import com.google.common.hash.Hashing;

import java.nio.charset.StandardCharsets;

public class SHAEncoding {

    public static String encodingwithSHA256(String inputString){
        String sha256hex = Hashing.sha256()
                .hashString(inputString, StandardCharsets.UTF_8).toString();
        return sha256hex;
    }
}
