import React, {Fragment} from 'react'
import PropTypes from 'prop-types'
import TextareaAutosize from 'react-autosize-textarea'
import he from 'he'
import get from 'lodash.get'
import shortid from 'shortid'

import Telepathic from '../../dist/Telepathic.js'
import {chatConfigTypes} from './Chat'

/* LOGIC */
const adminCodes = {
  name: '/name',
}

const emojiRegEx = /(:[\S]*:)/g
const boldRegEx = /(\*[\S]*\*)/g
const italicRegEx = /(_[\S]*_)/g
const strikeRegEx = /(~[\S]*~)/g
const codeRegEx = /(`[\S]*`)/g

const prefixClass = className => `telepathic-${className}`

const splitRichContent = string => {
  const emojiSplit = string.split(emojiRegEx)
  const emojiBold = emojiSplit.reduce((accum, item) => {
    const boldSplit = item.split(boldRegEx)
    return accum.concat(boldSplit)
  }, [])
  const emojiBoldItalic = emojiBold.reduce((accum, item) => {
    const italicSplit = item.split(italicRegEx)
    return accum.concat(italicSplit)
  }, [])
  const emojiBoldItalicStrike = emojiBoldItalic.reduce((accum, item) => {
    const strikeSplit = item.split(strikeRegEx)
    return accum.concat(strikeSplit)
  }, [])
  const richContent = emojiBoldItalicStrike.reduce((accum, item) => {
    const codeSplit = item.split(codeRegEx)
    return accum.concat(codeSplit)
  }, [])
  return richContent
}

class ChatBox extends React.Component {
  static propTypes = {
    handleBotResponse: PropTypes.func,
    emoji: PropTypes.objectOf(PropTypes.oneOfType([
      PropTypes.string.isRequired,
      PropTypes.shape({
        description: PropTypes.string,
        unicode: PropTypes.string,
      }).isRequired,
    ])).isRequired,
    config: PropTypes.shape(chatConfigTypes).isRequired,
    isBotPrivate: PropTypes.bool,
  }

  state = {
    messages: [],
    displayName: get(this.props.config, 'defaultDisplayName') || 'Guest',
    typedMsg: '',
    messageBoxId: `telepathic-messageBox-${shortid.generate()}`,
  }

  telepathicClient = Telepathic.start({
    linkId: get(this.props.config, 'telepathicLink'),
    onMessage: this.handleChatReceived,
  })

  handleChatReceived = receivedMsg => {
    const {messages, messageBoxId} = this.state
    // Update state with the new message
    this.setState({
      messages: [...messages, receivedMsg],
    }, () => {
      // Then scroll to the bottom of the message box to see the new message
      const messageBox = document.getElementById(messageBoxId)
      messageBox.scrollTop = messageBox.scrollHeight
    })
  }

  handleChangeMessage = e => {
    this.setState({typedMsg: e.currentTarget.value})
  }

  handleKeyUp = e => {
    const {typedMsg, messages, displayName, messageBoxId} = this.state
    const {handleBotResponse, isBotPrivate} = this.props

    /* RETURN KEY PRESSED */
    // If 'enter' was pressed without holding the 'shift' key,
    // and if there is actually a message typed in...
    if (e.key === 'Enter' && !e.shiftKey && typedMsg.trim()) {
      e.preventDefault()
      const messagesToAdd = [{userName: displayName, text: typedMsg}]
      const lowerCaseMessage = typedMsg.toLowerCase()
      let newDisplayName = displayName
      let isPrivateMessage = false

      /* ADMIN COMMANDS */
      // If the user typed the keyword to change their display name,
      // update it, accordingly
      if (lowerCaseMessage.includes(adminCodes.name)) {
        newDisplayName = typedMsg.replace(`${adminCodes.name} `, '').trim()
        // Update the user name in Telepathic
        this.telepathicClient.updateName(newDisplayName)
        messagesToAdd.push({
          userName: 'Notification',
          text: `Your name has been updated, ${newDisplayName}. Change it at any time by typing "/name your-new-name".`,
        })
        // Flag message to not send to Telepathic
        isPrivateMessage = true
      }

      /* BOT HANDLING */
      // If there is a bot, send the new message to it to determine its response
      const botResponse = handleBotResponse && handleBotResponse(typedMsg, messages)
      const botMessages = get(botResponse, 'messages')
      const botChangedDisplayName = get(botResponse, 'newDisplayName')
      // If there is a bot message, determine this exchange should be private
      if (botMessages) {
        isPrivateMessage = isBotPrivate
      }

      /* SUBMIT MESSAGE */
      // Send the new message to Telepathic, unless flagged as private
      !isPrivateMessage && this.telepathicClient.sendMessage(typedMsg)
      // Set state to show the new message and update if the display name has changed
      this.setState({
        typedMsg: '',
        messages: [...messages, ...messagesToAdd, ...botMessages],
        displayName: botChangedDisplayName || newDisplayName,
      }, () => {
        // Then scroll to the bottom of the message box to see the new message
        const messageBox = document.getElementById(messageBoxId)
        messageBox.scrollTop = messageBox.scrollHeight
      })
    }
  }

  findEmoji = emojiTag => {
    // Find the correct emoji by the raw emoji tag
    const foundEmoji = get(this.props.emoji, emojiTag.replace(/:/g, ''), {})
    // Format the found emoji in a specific shape
    return {
      src: (typeof foundEmoji === 'string') && foundEmoji,
      unicode: get(foundEmoji, 'unicode'),
      description: get(foundEmoji, 'description'),
    }
  }

  render () {
    const {messages, typedMsg, messageBoxId} = this.state
    const {config} = this.props
    const maxRows = get(config, 'textarea.maxRows') || 4
    const placeholder = get(config, 'textarea.placeholder') || 'Type something...'
    /* PRESENTATION */
    return (
      <div className={prefixClass('chatBox')}>
        <div className={prefixClass('messageBox')} id={messageBoxId}>
          <div className={prefixClass('messages')}>
            {messages.map((message, msgIndex) => {
              const paragraphs = message.text.trim().split(/[\n\r]/g)
              return (
                <Fragment key={`message-${msgIndex}`}>
                  <p className={prefixClass('userName')}><strong>{message.userName}</strong></p>
                  {paragraphs.map((paragraph, parIndex) => {
                    const richContent = splitRichContent(paragraph)
                    return (
                      <p className={prefixClass('message')} key={`paragraph-${parIndex}`}>
                        {richContent.map((contentItem, i) => {
                          // Emoji
                          if (contentItem.match(emojiRegEx)) {
                            const emoji = this.findEmoji(contentItem)
                            return (
                              <Fragment key={`${i}-emoji`}>
                                <img
                                  className={prefixClass('emoji')}
                                  display-if={get(emoji, 'src')}
                                  src={get(emoji, 'src')}
                                />
                                <span
                                  display-if={get(emoji, 'unicode')}
                                  role="img"
                                  aria-label={get(emoji, 'description')}
                                >
                                  {he.decode(`&#x${get(emoji, 'unicode')};`)}
                                </span>
                              </Fragment>
                            )
                            // Bold
                          } else if (contentItem.match(boldRegEx)) {
                            return <strong key={`${i}-bold`}>{contentItem.replace(/\*/g, '')}</strong>
                            // Italic
                          } else if (contentItem.match(italicRegEx)) {
                            return <em key={`${i}-italic`}>{contentItem.replace(/_/g, '')}</em>
                            // Strikethrough
                          } else if (contentItem.match(strikeRegEx)) {
                            return <s key={`${i}-strike`}>{contentItem.replace(/~/g, '')}</s>
                            // Code
                          } else if (contentItem.match(codeRegEx)) {
                            return <code key={`${i}-code`}>{contentItem.replace(/`/g, '')}</code>
                            // Text
                          } else {
                            return contentItem
                          }
                        })}
                      </p>
                    )
                  })}
                </Fragment>
              )
            })}
          </div>
        </div>
        <TextareaAutosize
          className={prefixClass('chatInput')}
          maxRows={maxRows}
          placeholder={placeholder}
          value={typedMsg}
          onKeyUp={this.handleKeyUp}
          onChange={this.handleChangeMessage}
        />
      </div>
    )
  }
}

export default ChatBox
