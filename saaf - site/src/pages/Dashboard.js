import NivelAgua from "./NivelAgua"
import NivelTurbidez from "./NivelTurbidez"
import styles from '../styles/Dashboard.module.css'
import { useEffect, useState } from "react"
import { getDatabase, ref, onValue, update} from "firebase/database";


export default function Dashboard() {

    const [estadoBomba, setEstadoBomba] = useState(false);

    useEffect(() => {
        const estadoBombaRef = ref(getDatabase());
        update(estadoBombaRef, {estado_bomba: estadoBomba})

    }, [estadoBomba]);
  return (<>
  <div className={styles.nav}>
    <div>
        <h2>SAA - Familiar</h2>
    </div>
    <h2>DASBOARD</h2>
  </div>
  <div className={styles.body}>
    <div className={styles.info}>

    <div className={styles.nivelagua}>

    <NivelAgua />
    </div>
    <div className={styles.nivelturbidez}>
    <button onClick={() => setEstadoBomba(!estadoBomba)}>LIGAR A BOMBA</button>
    <NivelTurbidez />

    </div>
    </div>
    </div>
    {console.log(estadoBomba)}
    
    </>
  )
}
