function update() {
    var select = document.getElementById("time");
    console.log(select);
    var option = select.options[select.selectedIndex];
    console.log(option);

    if (option.value == "empty") {
        document.getElementById("km").innerHTML = "0";
        document.getElementById("step").innerHTML = "0";
        document.getElementById("kcal").innerHTML = "0";
        return
    }


    fetch('/sessions/'+option.value)
    .then(response => response.json())
    .then(data => {
    console.log(data)
    document.getElementById("km").innerHTML = data[0];
    document.getElementById("step").innerHTML = data[1];
    document.getElementById("kcal").innerHTML = data[2];
    })
    }

function get_sessions() {
    fetch('/sessions')
    .then(response => response.json())
    .then(data => {
    var select = document.getElementById("time");
    for (const timestamp of data)
    {
        var option = document.createElement("option");
        option.value = timestamp;
        option.text = timestamp;
        select.appendChild(option);
    }

    });
}