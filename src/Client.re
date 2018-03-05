open WebSockets;

type t = {
  ws: WebSocket.t,
  linkId: string
};

type clientMessage = {. "userName": string, "text": string};

[@bs.module "shortid"] external generate : unit => string = "";

module Config = {
  type keys = {userName: string};
  let keys: keys = {userName: "telepathic:userName"};
};

/**
 * Choose a random name for a guest user
 */
let chooseName = () => "Guest (" ++ generate() ++ ")";

/**
 * Get the current userName from localStorage
 */
let getName = () => Dom.Storage.(localStorage |> getItem(Config.keys.userName));

/**
 * Change the current userName in localStorage
 */
let updateName = (~name: string) =>
  Dom.Storage.(localStorage |> setItem(Config.keys.userName, name));

/**
 * A convenience wrapper around `chooseName` that updates the userName in localStorage
 */
let makeName = () => {
  let name = chooseName();
  updateName(~name);
  name
};

/**
 * Retrieve the userName from localStorage if one is present, and make a userName if not
 */
let getOrCreateUserName = () => {
  let name = getName();
  switch name {
  | Some(name) => name
  | _ => makeName()
  }
};

/**
 * Send a message to the Slack channel
 * */
let sendMessage = (~linkId: string, ~text: string, client: t) => {
  open Actions;
  open Js.Json;
  let userName = getOrCreateUserName();
  let message = MessageSend(linkId, userName, text);
  client.ws |> WebSocket.sendString(message |> Encode.action |> stringify)
};

let receiveMessage = (~onMessage, event) => {
  open Actions;
  let json = event |> WebSockets.MessageEvent.stringData |> Js.Json.parseExn;
  switch (json |> Decode.action) {
  | Some(MessageReceive(userName, text)) => onMessage({"userName": userName, "text": text})
  /* Otherwise, ignore */
  | _ => ()
  }
};

/**
 * Register the linkId with the server so that we can receive messages from the associated channel
 */
let register = (client: t) => {
  open Actions;
  open Js.Json;
  let message = ClientRegister(client.linkId);
  client.ws |> WebSocket.sendString(message |> Encode.action |> stringify)
};

/**
 * Initialize a new Client
 * */
let make = (~url=?, ~linkId, ~onMessage: clientMessage => unit, ()) : t => {
  let ws = WebSocket.make(url |> Js.Option.getWithDefault("ws://telepathic.ecliptic.io"));
  let client = {ws, linkId};
  /* When connected, register the linkId with the server */
  ws
  |> WebSocket.on @@
  Open(() => register(client))
  |> WebSocket.on @@
  Message(receiveMessage(~onMessage))
  |> WebSocket.on @@
  Error((error) => Js.log2("WebSocket error: ", error))
  |> WebSocket.on @@
  Close((event) => Js.log2("WebSocket closed: ", CloseEvent.reason(event)))
  |> ignore;
  /* Return the client */
  client
};
