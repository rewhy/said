package cn.uprogrammer.sensordatacollect;

public class ActionTables {
    Action action1 = new Action();

    //main return an action while inputting a message
    public static Action GetAction(String id,byte[] data){
        Action result = new Action();


        switch (id) {
            case "070E":
                if(data[0]<=0x07&&data[1]==(byte)0x2F)
                {
                    if(data[2]==(byte) 0x09&&data[3]==(byte) 0x66){
                        if(data[4]==0x03&&data[5]==0x01){
                            result.ActionName = "Horn";
                            result.Value = 1;
                        }else if(data[4]==0x00){
                            result.ActionName = "Horn";
                            result.Value = 0;
                        }
                    }
                    else {
                        result = null;
                    }
                }
                else {
                    result = null;
                }
                break;
            case "0713":
                    result = null;
                break;
                default:
                    result = null;
        }


        return result;


    }


}



