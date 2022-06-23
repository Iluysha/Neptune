FROM python:3.10-slim

WORKDIR /server

ADD . /server

RUN apt-get update
RUN pip install google-api-python-client
RUN pip install google-cloud-iot
RUN pip install google-cloud-pubsub

EXPOSE 80

ENV NAME World
ENV GOOGLE_APPLICATION_CREDENTIALS=./google.json

CMD ["python", "server.py"]
