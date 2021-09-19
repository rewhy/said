package cn.uprogrammer.sensordatacollect;

import android.util.Log;

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class CanTables {
    private int RuleNum = 0;

    public static List<Rule> RuleTables = new ArrayList();

    public void InitCanTables(){
        Action a1 = new Action();
        a1.ActionName = "Horn";
        a1.Value = 1;
        Map testCondition1 = new HashMap();
        testCondition1.put("Vehicle_Speed","2110");
        Rule r0;
        r0 = CreatRule(ActionValue.REJECT,a1,testCondition1);
        AddRuleTable(r0);
        RuleNum++;


    }

    public Rule CreatRule(int decision, Action TAction, Map condition){
        Rule r0 = new Rule();
        r0.InitRule(decision,TAction,condition);
        return r0;
    }

    public int AddRuleTable(Rule r0){
        RuleTables.add(r0);
        RuleNum++;
        return RuleNum;
    }



    //Major function
    public int MakeDecision(Action a) {
        boolean flag;
        for (int i = 0;i < RuleTables.size(); i++){
            //every rule
            Rule temp = RuleTables.get(i);
            flag = true;
            if(ActionEqual(a,temp.TestAction)){
                //match action
                for(String key : temp.Conditions.keySet()){
                    //every state in PST(Conditon)
                    String condition = temp.Conditions.get(key);
                    //float ST_value = (float)Window.ST.SingleState.get(key);
                    double ST_value = StateTables.SingleState.get(key);
                    if(STMatch(condition,ST_value)==false){
                        flag = false;
                        break;
                    }
                }
                //循环结束
                if(flag) {
                    return temp.decision;
                }
            }
            else
                continue;

        }


        return ActionValue.MISMATCHING;
    }

    //考虑精度,compare condition
    public static boolean STMatch(String condition,double value){
        String relation = condition.substring(0,2);
        String s_value = condition.substring(2);
        double c_value = Double.parseDouble(s_value);
        boolean result=false;
        switch (relation){
            case ActionValue.EQUAL:
                result = Math.abs(c_value - value) < 0.1;
                break;
            case ActionValue.GREATER:
                result = value > c_value;
                break;
            case ActionValue.LESS:
                result = value < c_value;
                break;
            case ActionValue.GREATEREQUAL:
                result = value >= c_value - 0.1;
                break;
            case ActionValue.LESSEQUAL:
                result = value <= c_value + 0.1;
                break;
            case ActionValue.UNEQUAL:
                result = Math.abs(value - c_value) > 0.1;
                break;
                default:
                    Log.d("CanTable","rule has wrong");
        }
        return result;
    }

    public int Length(){
        return RuleNum;
    }

    //compare Action
    public boolean ActionEqual(Action a,Action b)
    {
        return a.ActionName == b.ActionName && a.Value == b.Value;
    }
}
