FROM node:alpine

# Default environment variables
ENV PORT=4000 \
    NODE_ENV=production

# Create app directory
RUN mkdir -p /srv/telepathic
WORKDIR /srv/telepathic

# Install app dependencies
COPY package.json /srv/telepathic
RUN npm install

# Copy app source
COPY . /srv/telepathic

EXPOSE $PORT
CMD ["npm", "start"]
