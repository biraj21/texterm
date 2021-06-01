// program to calculate factorial

/**
 * 
 * @param {*} n 
 * @returns {*} n!
 */
class TMath{

    fact(n){
        if(n==0){
            return 1;
        } 
        else{
            return n*fact(n-1);
        }
    }

}

const number=10;
var math=new TMath();
let result=math.fact(number);

console.log(result);

