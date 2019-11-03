import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;

/**
 * Created by Rene Argento on 19/07/17.
 */
public class Exercise4Mod 
{

    public static void main(String[] args) 
    {
        Exercise4Mod exercise4 = new Exercise4Mod();
        int[] values = exercise4.perfectHashFunction();

        if (values != null) 
        {
            System.out.println("a = " + values[0]);
            System.out.println("m = " + values[1]);
        }
    }

    private int[] perfectHashFunction() 
    {
        int[] values = new int[2];

        String[] names10  = {"BritCo", "Albert", "Saskat", "Manito", "Ontari", "Quebec", "NewLab", "NeBrun", "NoScot",
            "PrinEd"};                                                                                                      

        //String[] names7 = {"BritCo", "Albert", "Saskat", "Manito", "Ontari", "Quebec", "NewLab"};
        //String[] namesRand = {"YaLiWA", "OStREn", "qsiaTL", "PqbCvs", "ZAIore", "AgEEwq", "hjIcxM", "rtivol", "xwuqoH", "ZoKvwe"};

        for(int m = 2; m <= 100; m++) {
            for(int a = 1; a <= 1000; a++) {
                Set<Integer> hashes = new HashSet<>();

                //for(int i = 0; i < names.length; i++) 
                //for(int i = 0; i < names.length; i++) 
                for(int i = 0; i < names10.length; i++) 
                {
                    ArrayList sum_hash = hashCodeFunction(a, names10[i], m);
                    if(hashes.add((Integer)sum_hash.get(1)))
                    {
                        System.out.println("Name: " + sum_hash.get(2));
                        System.out.println("Sum: " +  sum_hash.get(0));
                        System.out.println("Hash: " +  sum_hash.get(1));
                    }
                }

                if (hashes.size() == 10) {
                    //Perfect hash function found
                    values[0] = a;
                    values[1] = m;
                    return values;
                }
            }
        }

        return values;
    }

    private ArrayList hashCodeFunction(int a, String k, int m) 
    {
      ArrayList retval = new ArrayList(3);
      retval.ensureCapacity(3);
      System.out.println("retval size: " + retval.size()); 
      int sum = 0;
      for (int i = 0; i < k.length(); i++)
      {
        char c = k.charAt(i); 
        sum += c; 
      }
      int hash = (a * sum) % m;
      retval.add(0, sum);
      retval.add(1, hash); 
      retval.add(2, k);
      return retval; 
    }

}
