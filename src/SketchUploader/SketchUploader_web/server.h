#include "Arduino.h"

const char server_html[] PROGMEM = R"rawliteral(
    <!DOCTYPE HTML>
    <html>
    <head>
        <title>SketchUploader</title>
        <meta name='viewport' content='width=device-width, initial-scale=1'>
        <link rel='icon' href='data:,'>
        <script src='http://code.jquery.com/jquery-1.11.1.min.js'></script>

        <style> 
            textarea {
              width: 100%;
              padding: 12px 20px;
              margin: 8px 0;
              box-sizing: border-box;
              background-color: black;
              font-family: Courier New;
              color: white;
              overflow-y: scroll;
            }
        </style>
    </head>

    <body>
        <h2>ESP32 Sketch Uploader - Riego Automata v2.0</h2>
        <h3>By @dabecart</h3>

        <button onClick='logoutButton(this)'>Logout</button>

        <br><br>

        <form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>
            <input type='file' name='update' id='file_upload'>
            <input type='submit' value='Update'>
        </form>
        <div id='prg'>Progress: 0%</div>

        <br>

        <label for='console'>Console</label>
        <textarea id='console' name='console' rows='12' cols='50'></textarea>
        
        <div style='width: 100%'>
            <div style="display: table-cell; width: 20%">
                <input type='checkbox' id='show_date' checked>
                <label for='show_date'>Show timestamps</label>
            </div>

            <div style="display: table-cell; width: 20%">
                <input type='checkbox' id='follow_last_line' checked>
                <label for='follow_last_line'>Follow last line</label>
            </div>
        </div>

        <button onClick='fetchLastLogs()'>Fetch last logs</button>
    

        <script>
            $('#file_upload').change(function() {
                if($(this).val().split('.').pop().toLowerCase() != 'bin'){
                    alert('File is not a binary (.bin)!');
                }
            });

            $('form').submit(function(e){
                if($('#file_upload').val().split('.').pop().toLowerCase() != 'bin'){
                    alert('Cannot upload this file!');
                    return false;
                }

                e.preventDefault();
                var form = $('#upload_form')[0];
                var data = new FormData(form);
                $.ajax({
                    url: '/update',
                    type: 'POST',
                    data: data,
                    contentType: false,
                    processData:false,
                    xhr: function() {
                        var xhr = new window.XMLHttpRequest();
                        xhr.upload.addEventListener('progress', function(evt) {
                            if (evt.lengthComputable) {
                                var per = Math.round(evt.loaded*100/evt.total);
                                var text = $('#prg');
                                text.html('Progress: ' + per + '%');
                                if(evt.loaded == evt.total){
                                    doneUploading();
                                } 
                            }
                        }, false);
                        return xhr;
                    },
                    
                    success:function(d, s) {
                        doneUploading();
                    },
                
                    error: function (a, b, c) {}
                });
            });

            function doneUploading(){
                $('#prg').html('Progress: 100% (Done!)');
                console.log('success!')
                setTimeout(function(){
                    window.location.reload();
                }, 5000);
            }
        </script>

        <script>
            function logoutButton(elem){
                var xhr = new XMLHttpRequest();
                xhr.open('GET', '/logout', true);
                xhr.send();

                elem.innerHTML = 'Logged Out!';
                elem.disabled = true;
            }

            function fetchLastLogs(){
                var xhr = new XMLHttpRequest();
                xhr.open('POST', '/fetchLastLogs', true);
                xhr.send();
            }

            if(!!window.EventSource){
                var source = new EventSource('/events');

                source.addEventListener('open', function(e) {
                  console.log('Events Connected');
                }, false);

                source.addEventListener('error', function(e) {
                  if (e.target.readyState != EventSource.OPEN) {
                    console.log('Events Disconnected');
                  }
                }, false);

                source.addEventListener('message', function(e) {
                  console.log('message', e.data);
                }, false);

                source.addEventListener('console', function(e) {
                    var console = document.getElementById('console');
                    var data = e.data;

                    if(!document.getElementById('show_date').checked){
                        var splitFrom = data.indexOf('>');
                        data = data.substring(splitFrom, data.length);
                    }
                    console.value += data + '\n';
                    if(document.getElementById('follow_last_line').checked){
                        console.scrollTop = console.scrollHeight;
                    }
                }, false);
            }
        </script>
    </body>
    </html>
)rawliteral";