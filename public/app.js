var nameArrayElements = ['led_led', 'led_presence', 'led_brightness', 'led_mode', 'water_wateringinfo', 'water_lastwatering', 'water_time', 'water_alarm_1'];

$(document).ready(function(){
    for(let i = 0; i < nameArrayElements.length; i++){
        let name = nameArrayElements[i];
        
        let split = name.indexOf('_');
        let id1 = name.substring(0, split);
        let id2 = name.substring(split+1, name.length);
        let serverPath = id1 + '/' + id2;
    
        database.ref(serverPath).on('value', (snapshot) => {
            let element = document.getElementsByName(nameArrayElements[i])[0];
            
            let value = snapshot.val();
            console.log("Fetching from " + serverPath + " Value=" + value);
            if(element.value == ""){
                element.name = value;
            }else if(element.id == 'span'){
                element.innerHTML = value;
            }else{
                if(element.type == "checkbox"){
                    //element.checked = value;
                    $('#' + nameArrayElements[i]).prop("checked", value).checkboxradio("refresh");
                    disableAlarmByName(nameArrayElements[i], value);
                }else{
                    if(element.value == "true" || element.value == "false"){
                        $('input[value=true][name=' + nameArrayElements[i] + ']').prop("checked", value);  // This should be the On button.
                        $('input[value=false][name=' + nameArrayElements[i] + ']').prop("checked", !value);  // This should be the On button.
                        $("input[type='radio']").checkboxradio("refresh");
                    }else{
                        element.value = value;
                        if(element.type == 'number'){ // This is a slider. 
                            $('#'+nameArrayElements[i]).slider('refresh');
                        }
                    }
                }   
            }
        }, (errorObject) => {
            console.log('The read failed: ' + errorObject.name);
        });
    }
});

function disableAlarmByName(name, checked){
    if(checked) $('.timepicker[name='+name+']').parent().show('normal');
    else $('.timepicker[name='+name+']').parent().hide('normal');
    //el.parentNode.style.visibility = x.checked?"visible":"hidden";
}

function sendToServer(x){
    var split = x.name.indexOf('_');
    var id1 = x.name.substring(0, split);
    var id2 = x.name.substring(split+1, x.name.length);

    if(x.value == ""){ // For single buttons
        firebase.database().ref(id1).child(id2).set(x.name);
    }else{
        if(x.type == "checkbox"){
            firebase.database().ref(id1).child(id2).set(x.checked);
        }else{
            if(x.value == "true"){
                console.log(x.name);
                firebase.database().ref(id1).child(id2).set(true);
            }else if(x.value == "false"){
                firebase.database().ref(id1).child(id2).set(false);
            }else{
                firebase.database().ref(id1).child(id2).set(x.value);
            }
        }   
    }
}