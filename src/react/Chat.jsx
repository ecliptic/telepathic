import React from 'react'
import PropTypes from 'prop-types'
import get from 'lodash.get'

import Bot, {botPropTypes} from './Bot'
import ChatBox from './ChatBox'

/* LOGIC */
const loadCSS = () => {
  const cssId = 'telepathic-chat-css'
  // Check if the css has already been loaded
  if (!document.getElementById(cssId)) {
    const head = document.getElementsByTagName('head')[0]
    const link = document.createElement('link')
    link.id = cssId
    link.rel = 'stylesheet'
    link.type = 'text/css'
    link.href = './chat.css'
    link.media = 'all'
    head.appendChild(link)
  }
}

export const chatConfigTypes = {
  telepathicLink: PropTypes.string.isRequired,
  defaultDisplayName: PropTypes.string,
  textarea: PropTypes.shape({
    maxRows: PropTypes.number,
    placeholder: PropTypes.string,
  }),
}

class Chat extends React.Component {
  static propTypes = {
    isStyled: PropTypes.bool,
    slackLegacyToken: PropTypes.string,
    bot: PropTypes.shape(botPropTypes),
    ...chatConfigTypes,
  }

  static defaultProps = {
    isStyled: true,
  }

  state = {
    emoji: {},
  }

  componentDidMount () {
    const {isStyled, slackLegacyToken} = this.props
    // Load the CSS into the DOM if the component is supposed to be "styled"
    isStyled && loadCSS()

    // Load Slack unicode emoji data into state
    this.initializeUnicodeSlackEmoji()

    // Load custom Slack emoji urls into state if a legacy token from Slack was provided
    slackLegacyToken && this.initializeCustomSlackEmoji()
  }

  initializeCustomSlackEmoji = token => {
    let httpRequest = new XMLHttpRequest()
    if (!httpRequest) {
      throw new Error('Could not make an XMLHttpRequest for the custom Slack emoji.')
    } else {
      httpRequest.onreadystatechange = () => {
        // If the request has finished...
        if (httpRequest.readyState === XMLHttpRequest.DONE) {
          // And the status is good...
          if (httpRequest.status === 200) {
            // Parse the JSON and save the emoji data to state
            const data = JSON.parse(httpRequest.responseText)
            const customEmojiData = get(data, 'emoji')
            this.setState({
              emoji: {
                ...this.state.emoji,
                ...customEmojiData,
              },
            })
          } else {
            // Otherwise, throw an error
            throw new Error('There was an issue with the custom Slack emoji request.')
          }
        }
      }
      httpRequest.open('GET', `https://slack.com/api/emoji.list?token=${token}`)
      httpRequest.send()
    }
  }

  initializeUnicodeSlackEmoji = () => {
    let httpRequest = new XMLHttpRequest()
    if (!httpRequest) {
      throw new Error('Could not make an XMLHttpRequest for the unicode Slack emoji.')
    } else {
      httpRequest.onreadystatechange = () => {
        // If the request has finished...
        if (httpRequest.readyState === XMLHttpRequest.DONE) {
          // And the status is good...
          if (httpRequest.status === 200) {
            // Parse the JSON
            const data = JSON.parse(httpRequest.responseText)
            const emojiJson = data || []
            // Combine the data into a new object organized by the emoji short names
            const newEmoji = emojiJson.reduce((emojiAccum, emojiObject) => {
              const shortNames = get(emojiObject, 'short_names')
              const shortNameObjects = shortNames.reduce((shortNameAccum, shortName) => ({
                ...shortNameAccum,
                [shortName]: {
                  description: emojiObject.name,
                  unicode: emojiObject.unified,
                },
              }), {})
              return {
                ...emojiAccum,
                ...shortNameObjects,
              }
            }, {})
            // Save this emoji data to local state
            this.setState({
              emoji: {
                ...this.state.emoji,
                ...newEmoji,
              },
            })
          } else {
            // Otherwise, throw an error
            throw new Error('There was an issue with the unicode Slack emoji request.')
          }
        }
      }
      httpRequest.open('GET', 'https://raw.githubusercontent.com/iamcal/emoji-data/master/emoji.json')
      httpRequest.send()
    }
  }

  render () {
    const {emoji} = this.state
    const {bot, telepathicLink, defaultDisplayName, textarea} = this.props
    const config = {
      telepathicLink,
      defaultDisplayName,
      textarea,
    }

    if (bot) {
      const {isPrivate, ...botConfig} = bot
      /* PRESENTATION */
      return (
        <Bot
          {...botConfig}
          render={({handleBotResponse}) => (
            <ChatBox
              handleBotResponse={handleBotResponse}
              isBotPrivate={isPrivate}
              emoji={emoji}
              config={config}
            />
          )}
        />
      )
    } else {
      return <ChatBox emoji={emoji} config={config} />
    }
  }
}

export default Chat
