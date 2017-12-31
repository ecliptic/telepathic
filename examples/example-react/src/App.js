import React, {Component} from 'react'
import logo from './logo.svg'
import './App.css'
import * as Client from 'telepathic/src/client/Client'

const CHAT_URL = 'ws://localhost:4000/chat'
const LINK_ID = '6b6188b4-c124-473e-b430-ae0a20c36c8b'

class App extends Component {
  constructor (...args) {
    super(...args)
    this.client = Client.start({linkId: LINK_ID, url: CHAT_URL})
    this.state = {
      nickname: this.client.getOrCreateUserName(),
      chat: {
        messages: [], // {text: '', userName: ''}
      },
      message: '',
    }
  }

  handleUpdateNickname = event => {
    const nickname = event.target.value
    this.client.updateName(nickname)
    this.setState({nickname})
  }

  handleUpdateMessage = event => {
    const message = event.target.value
    this.setState({message})
  }

  handleSend = event => {
    event.preventDefault()
    this.client.sendMessage(this.state.message)
    const message = {text: this.state.message, userName: this.state.nickname}
    this.setState({
      message: '',
      chat: {messages: this.state.chat.messages.concat([message])},
    })
  }

  render () {
    return (
      <div className="App">
        <header className="App-header">
          <img src={logo} className="App-logo" alt="logo" />
          <h1 className="App-title">Welcome to the Telepathic test page!</h1>
        </header>
        <p className="App-intro">Let's chat!</p>
        <div className="App-chat">
          {this.state.chat.messages.map((message, i) => (
            <p key={`chat:message:${i}`}>
              <em>{message.userName}:</em> {message.text}
            </p>
          ))}
        </div>
        <form onSubmit={this.handleSend}>
          <p>
            <label>Nickname: </label>
            <input
              type="text"
              id="update-name"
              value={this.state.nickname}
              onChange={this.handleUpdateNickname}
            />
          </p>
          <p>
            <label htmlFor="send-message">Message: </label>
            <input
              type="text"
              id="send-message"
              value={this.state.message}
              onChange={this.handleUpdateMessage}
            />
            <button type="submit">Send</button>
          </p>
        </form>
      </div>
    )
  }
}

export default App
