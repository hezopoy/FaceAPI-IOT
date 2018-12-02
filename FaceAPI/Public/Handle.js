function setKey() {
    var subscriptionKey = document.getElementById("keyAPI").value;
    if (subscriptionKey == "") {
        //alert("Key is null!!");
        document.getElementById("keyAPI").style.borderColor = "red";
        document.getElementById("keyAPI").focus();
    } else {
        document.getElementById("keyAPI").style.borderColor = document.getElementById("inputImage").style.borderColor;
    }
}
function processImage() {
    // Replace <Subscription Key> with your valid subscription key.
    //"406352e4772f444c934e8f8c93cedd94"
    var subscriptionKey = document.getElementById("keyAPI").value;

    // NOTE: You must use the same region in your REST call as you used to
    // obtain your subscription keys. For example, if you obtained your
    // subscription keys from westus, replace "westcentralus" in the URL
    // below with "westus".
    //
    // Free trial subscription keys are generated in the westcentralus region.
    // If you use a free trial subscription key, you shouldn't need to change 
    // this region.
    var uriBase =
        "https://westcentralus.api.cognitive.microsoft.com/face/v1.0/detect";

    // Request parameters.
    var params = {
        "returnFaceId": "true",
        "returnFaceLandmarks": "false",
        "returnFaceAttributes":
            "age,gender,headPose,smile,facialHair,glasses,emotion," +
            "hair,makeup,occlusion,accessories,blur,exposure,noise"
    };

    // Display the image.
    var sourceImageUrl = document.getElementById("inputImage").value;
    document.querySelector("#sourceImage").src = sourceImageUrl;

    // Perform the REST API call.
    $.ajax({
        url: uriBase + "?" + $.param(params),

        // Request headers.
        beforeSend: function (xhrObj) {
            xhrObj.setRequestHeader("Content-Type", "application/json");
            xhrObj.setRequestHeader("Ocp-Apim-Subscription-Key", subscriptionKey);
        },

        type: "POST",

        // Request body.
        data: '{"url": ' + '"' + sourceImageUrl + '"}',
    })

        .done(function (data) {
            // Show formatted JSON on webpage.
            $("#responseTextArea").val(JSON.stringify(data, null, 2));
            var numberOfPeople = data.length;
            var male = 0;
            var female = 0;
            var anger = 0;
            var happy = 0;
            var neutral = 0;
            var surprise = 0;
			var result = 0;
            for (var i = 0; i < data.length; i++) {
                data[i].faceAttributes.gender == 'male' ? male++ : female++;
                if (data[i].faceAttributes.emotion.anger >= 0.5) {
                    anger++;
                } else if (data[i].faceAttributes.emotion.happiness >= 0.5) {
                    happy++;
                } else if (data[i].faceAttributes.emotion.neutral >= 0.5){
                    neutral++;
                } else if (data[i].faceAttributes.emotion.surprise >= 0.5){
                    surprise++;
                }
            }
            document.getElementById("numberOfPeople").innerHTML = numberOfPeople;
            document.getElementById("male").innerHTML = male;
            document.getElementById("female").innerHTML = female;
            document.getElementById("happy").innerHTML = happy;
            document.getElementById("anger").innerHTML = anger;
            document.getElementById("neutral").innerHTML = neutral;
            document.getElementById("surprise").innerHTML = surprise;
			
			if(happy/numberOfPeople >= 0.5){
				result = 3;  
			}else{
				if(neutral/numberOfPeople < 0.7){
					if((happy > anger) || (surprise > anger)){
						result = 3;  
					}else{
						result = 1; 
					}
				}else{
					result = 2;  
				}
			}
			var ketluan = "";
			if(result == 1)
					ketluan = "Không tốt";
			if(result == 2)
					ketluan = "Bình thường";
			if(result == 3)
					ketluan = "Tốt";
				
			document.getElementById("result").innerHTML = ketluan;
            var url = 'https://api.thingspeak.com/update?api_key=DGFMV20NVMZDBSAU&field7=' + numberOfPeople + '&field1=' + male + '&field8=' + result + '&field2=' + female + '&field3=' + anger + '&field4=' + happy + '&field5=' + neutral + '&field6=' + surprise;

            var xhr = new XMLHttpRequest();
            xhr.open("POST", url, true);

            xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

            xhr.onreadystatechange = function () {
                if (this.readyState === XMLHttpRequest.DONE && this.status === 200) {
                    var json = JSON.parse(xhr.responseText);
                }
            }
            xhr.send();
        })

        .fail(function (jqXHR, textStatus, errorThrown) {
            // Display error message.
            var errorString = (errorThrown === "") ?
                "Error. " : errorThrown + " (" + jqXHR.status + "): ";
            errorString += (jqXHR.responseText === "") ?
                "" : (jQuery.parseJSON(jqXHR.responseText).message) ?
                    jQuery.parseJSON(jqXHR.responseText).message :
                    jQuery.parseJSON(jqXHR.responseText).error.message;
            alert(errorString);
        });
};