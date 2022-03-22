function update() {
    var select = document.getElementById("session");
    console.log(select);
    var option = select.options[select.selectedIndex];
    console.log('OPTION:')
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
        document.getElementById("km").innerHTML = data[1] + " (km)";
        document.getElementById("step").innerHTML = data[2] + " (steps)";
        document.getElementById("kcal").innerHTML = Number.parseFloat(data[3]).toFixed(2) + " (kcal)";
    })
}

function get_sessions() {
    fetch('/sessions')
    .then(response => response.json())
    .then(sessions => {
        console.log(sessions)
        var select = document.getElementById("session");
        for (const sess of sessions)
        {
            var option = document.createElement("option");
            option.value = sess[0];
            option.text = "Session: " + sess[0];
            select.appendChild(option);
        }
    });
}

function deleteSession() {
    var select = document.getElementById("session");
    var option = select.options[select.selectedIndex];

    if (option.value !== "empty") {
        fetch('/sessions/' + option.value + '/delete')
        .then(response => response.json())
        .then(r => {
            console.log(r);
        })
        console.log("Deleting sesssion!")
    }
}