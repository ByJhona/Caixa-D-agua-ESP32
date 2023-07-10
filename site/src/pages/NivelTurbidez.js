import { Chart } from "react-google-charts";
import {database, app} from '../services/firebase'
import { getDatabase, ref, onValue} from "firebase/database";
import React, { useEffect, useState } from "react";


export const dataDefault = [
    ["Titulo", "Valor"],
  ["Turbidez", 0]
  
];

export const options = {
    width: "100%",
    height: "100%",
    redFrom: 90,
    redTo: 100,
    yellowFrom: 65,
    yellowTo: 90,
    minorTicks: 3,
};

export default function NivelTurbidez() {
  
  const [nivelTurbidez, setNivelTurbidez] = useState([]);
  

  useEffect(() => {
    
    const nivelTurbidezRef = ref(getDatabase(), 'nivel_turbidez/');
    onValue(nivelTurbidezRef, (snapshot) => {
      const snap = snapshot.val();
      if(!snapshot.exists()) {
        setNivelTurbidez(dataDefault);
        return;
      };
      console.log(snap);
      setNivelTurbidez([["Titulo", "Valor"],["Turbidez", snap]]);
      
      
    });
  }, []);
    return (
      <>
       
    
    <Chart
      chartType="Gauge"
      width="100%"
      height="100%"
      data={nivelTurbidez}
      options={options}
    />
  
      </>

    )
  }
  