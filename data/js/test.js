/**
 * Mock function
 */

async function testRender(){

    let target = document.getElementById("cpu-data");
    let innerTarget = target.querySelector("div");
    let innerTarget2 = innerTarget.querySelector(".value");
    while(true)
    {
        await new Promise(resolve => setTimeout(resolve, 1000));
        var rand = Math.floor(Math.random() * (100 - 0 + 1) + 0);
        innerTarget2.innerHTML = rand + "%";
    }
}

testRender();