FROM python:3.9-alpine

WORKDIR /home/devman

RUN apk add gcc libc-dev g++
RUN pip install pipenv
COPY Pipfile* ./
RUN pipenv install --system

COPY . .

USER 1337:1337
ENTRYPOINT [ "python3", "devman.py"]
