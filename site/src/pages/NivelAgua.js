import { Chart } from "react-google-charts";
import { database, app } from '../services/firebase'
import { getDatabase, ref, onValue } from "firebase/database";
import React, { useEffect, useState } from "react";


export const dataDefault = [
  ["Tempo em segundos", "Nível de água"],
  ['00-00-00 00:00:00', 0],

];

export const options = {
  title: "Nivel de Agua no Reservatorio",
  hAxis: {
    title: "Tempo(Dias)", titleTextStyle: { color: "#333" },
    viewWindow: {
      min: 0
    }
  },
  vAxis: {
    title: "Litros(L)", minValue: 0,
    viewWindow: {
      max: 100,
      min: 0
    }
  },
  chartArea: { width: "80%", height: "60%" },

};

export default function NivelAgua() {

  const [nivelAgua, setNivelAgua] = useState([]);
  const [nivelAguaHistorico, setNivelAguaHistorico] = useState([]);

  const [comeco, setComeco] = useState(0);
  const [fim, setFim] = useState(0);
  const [historico, setHistorico] = useState(false);



  useEffect(() => {

    if (comeco < fim) {
      setNivelAguaHistorico([["Tempo em segundos", "Nível de água"]]);
            console.log(comeco);
      console.log(fim)
      
      nivelAgua.map((dado, ind) => {
        if (ind >= comeco && ind <= fim) {
          console.log("teste")
          setNivelAguaHistorico((ted) => [...ted, dado]);
          
        }
        

      })
    }

  }, [comeco, fim]);

  useEffect(() => {

    const nivelAguaRef = ref(getDatabase(), 'nivel_agua/');
    onValue(nivelAguaRef, (snapshot) => {
      const snap = snapshot.val();
      if (!snapshot.exists()) {
        setNivelAgua(dataDefault);
        return;
      };

      if (!historico) {
        setNivelAgua([["Tempo em segundos", "Nível de água"]]);


        for (const [key, value] of Object.entries(snap)) {

          setNivelAgua((dado) => [...dado, [key, value]]);


        }
      }


    });
  }, []);
  return (
    <>

      <form >
        <input type="checkbox" onChange={() => setHistorico(!historico)} />


        <select onChange={(data) => setComeco(data.target.value)}>
          {nivelAgua.map((dado, ind) => {
            return <option value={ind} key={ind}>{dado[0]}</option>
          })}

        </select>

        <select onChange={(data) => setFim(data.target.value)}>
          {nivelAgua.map((dado, ind) => {
            return <option value={ind} key={ind}>{dado[0]}</option>
          })}

        </select>
      </form>

      {console.log(historico)}
      <Chart


        chartType="AreaChart"
        width="100%"

        height="100%"
        data={historico ? nivelAguaHistorico : nivelAgua}
        options={options}
      />

    </>

  )
}
