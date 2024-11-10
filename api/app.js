import express from 'express'
import pg from 'pg'
const { Pool } = pg
import  mqtt from 'mqtt'
import configureEndpoints from './endpoints.js'
import configureMqttEvents from './mqtt-events.js'
import dotenv from 'dotenv'
dotenv.config()

const pool = new Pool({
  connectionString: process.env.DATABASE_URL
});
const app = express();
app.use(express.json());
const mqttUrl = process.env.MQTT_URL || 'ws://localhost:8081';
const client = mqtt.connect(mqttUrl);
configureEndpoints(app, client, pool)
configureMqttEvents(client, pool)

const PORT = 3000;
app.listen(PORT, () => {
  console.log(`Servidor rodando na porta ${PORT}`);
});

