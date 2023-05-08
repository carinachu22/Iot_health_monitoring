// At 5.10pm it is able to CRUD on firebase 
// At 1.30am It is able to CRUD, and display data realtime in table format 
// At 6am, it is able to do everything, but cannot display data -> most probably because of the nested child. Find a way to access it -> Ans is by using onChildAdded
// At 5pm, it is able to display data on every new child added to the firebase -> Next step is to upload real values to firebase 
import {initializeApp} from  "https://www.gstatic.com/firebasejs/9.9.3/firebase-app.js";
import { getDatabase, ref, set,get,child,remove,onValue, onChildAdded,
        query,limitToFirst,limitToLast,orderByChild,orderByKey,startAt,startAfter,endAt,endBefore,equalTo} from  "https://www.gstatic.com/firebasejs/9.9.3/firebase-database.js";

const firebaseApp = initializeApp({
  apiKey: "AIzaSyDAtJkZ4hg-ppp5XylD3EiMTMZJyLWh4WE",
  authDomain: "helmet-v3.firebaseapp.com",
  databaseURL: "https://helmet-v3-default-rtdb.asia-southeast1.firebasedatabase.app",
  projectId: "helmet-v3",
  storageBucket: "helmet-v3.appspot.com",
  messagingSenderId: "1056162376374",
  appId: "1:1056162376374:web:0e717808c7960e0b1ad417"
});
 
const db = getDatabase();
// 6 sensor readings elements
var tempElement = document.getElementById("temp");
var humElement = document.getElementById("hum");
var bodytempElement = document.getElementById("bodytemp");
var heartElement = document.getElementById("heart");
var spo2Element = document.getElementById("spo2");
var fallElement = document.getElementById("fall");

function epochToJsDate(epochTime){
  return new Date(epochTime*1000);
}

function epochToDateTime(epochTime){
  var epochDate = new Date(epochToJsDate(epochTime));
  var time = 
    ("00" + epochDate.getHours()).slice(-2) + ":" +
    ("00" + epochDate.getMinutes()).slice(-2) + ":" +
    ("00" + epochDate.getSeconds()).slice(-2);

  return time;
}

function GetNewDataRealtime2(){
  const que= query(ref(db,"UsersData/Jg7xrtdPCQSALVj5sWt4DZ4X7tF3/readings"),orderByKey(),limitToLast(1));
  onChildAdded(que, (snapshot)=>{
    console.log(snapshot.val());
    var jsonData = snapshot.toJSON()
    var temperature = jsonData.temperature;
    var humidity = jsonData.humidity;
    var bodytemperature = jsonData.bodytemperature;
    var heartrate = jsonData.heartrate;
    var spo2 = jsonData.spo2;
    var fall = jsonData.fall;
    var timestamp = epochToDateTime(jsonData.timestamp);
    tempElement.innerHTML=temperature;
    humElement.innerHTML=humidity;
    bodytempElement.innerHTML=bodytemperature;
    heartElement.innerHTML=heartrate;
    spo2Element.innerHTML=spo2;
    fallElement.innerHTML=fall;

    addData(bodyTemperatureChart,timestamp,bodytemperature);
    addData(humidityChart,timestamp,humidity);
    addData(temperatureChart,timestamp,temperature);
    
  });
}

const data_bodytemperature = {
labels: [],
datasets: [
    {
    label: "Forehead temperature",
    data: [],
    backgroundColor: "rgb(255, 99, 132)",
    borderColor: "rgb(255, 99, 132)",
    },
],
};

const data_humidity = {
  labels: [],
  datasets: [
      {
      label: "Humidity",
      data: [],
      backgroundColor: "rgb(255, 99, 132)",
      borderColor: "rgb(255, 99, 132)",
      },
  ],
  };

  const data_temperature = {
    labels: [],
    datasets: [
        {
        label: "Temperature",
        data: [],
        backgroundColor: "rgb(255, 99, 132)",
        borderColor: "rgb(255, 99, 132)",
        },
    ],
    };
// Config block
const config_bodytemperature = {
type: "line",
data: data_bodytemperature,
options: {},
};

const config_humidity = {
  type: "line",
  data: data_humidity,
  options: {},
  };

  const config_temperature = {
    type: "line",
    data: data_temperature,
    options: {},
    };
//render block
const bodyTemperatureChart = new Chart(document.getElementById("bodyTemperatureChart"), config_bodytemperature);
const humidityChart = new Chart(document.getElementById("humidityChart"), config_humidity);
const temperatureChart = new Chart(document.getElementById("temperatureChart"), config_temperature);

function addData(chart,label,data){
    chart.data.labels.push(label);
    chart.data.datasets.forEach((dataset) => {
      console.log(data)
        dataset.data.push(data);
    });
    chart.update();
}

window.onload =GetNewDataRealtime2;


