package cn.uprogrammer.sensordatacollect;

import java.util.HashMap;
import java.util.Map;

public class Rule {
    public int decision;

    public Action TestAction = new Action();

    public Map<String,String> Conditions = new HashMap();

    public void InitRule(int d0,Action a0,Map m0)
    {
        this.decision = d0;
        this.TestAction = a0;
        this.Conditions = m0;
    }


    public void AddCondition(String StateName,String condition){
        Conditions.put(StateName,condition);
    }

    public Object RemoveConditon(String StateName){
        return Conditions.remove(StateName);
    }

    public int SetDecidon(int value){
        decision = value;
        return decision;
    }

    public int GetDecision() {
        return decision;
    }



}
