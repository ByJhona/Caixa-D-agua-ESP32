// Import the functions you need from the SDKs you need
import { initializeApp } from "firebase/app";
import {getDatabase} from "firebase/database"
// TODO: Add SDKs for Firebase products that you want to use
// https://firebase.google.com/docs/web/setup#available-libraries

// Your web app's Firebase configuration
// For Firebase JS SDK v7.20.0 and later, measurementId is optional
const firebaseConfig = {
  apiKey: "AIzaSyARa9LjX-_oE7nGRihgHg893UEw2YNAyGw",
  authDomain: "caixa-d-agua-esp32.firebaseapp.com",
  databaseURL: "https://caixa-d-agua-esp32-default-rtdb.firebaseio.com",
  projectId: "caixa-d-agua-esp32",
  storageBucket: "caixa-d-agua-esp32.appspot.com",
  messagingSenderId: "92842414408",
  appId: "1:92842414408:web:58df5141ef7fb6e331637d",
  measurementId: "G-F39VKRK1FD"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
export default {app};