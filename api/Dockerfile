# Usar a imagem base do Node.js
FROM node:18

# Criar diretório de trabalho
WORKDIR /usr/src/app

# Copiar package.json e package-lock.json
COPY package*.json ./

# Instalar dependências
RUN npm install

# Copiar o restante da aplicação
COPY . .

# Expor a porta 3000 (a porta do servidor Express)
EXPOSE 3000

# Comando para iniciar a aplicação
CMD ["node", "app.js"]
